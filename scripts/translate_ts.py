#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import requests

DOCTYPE = '<!DOCTYPE TS>'


LANGUAGE_CODE_MAP = {
    "en_US": "en",
    "zh_CN": "zh-CN",
    "zh_TW": "zh-TW",
    "ja_JP": "ja",
    "ko_KR": "ko",
    "de_DE": "de",
    "fr_FR": "fr",
    "es_ES": "es",
    "ru_RU": "ru",
}


MASK_TOKENS = (
    ("%1", "__PERCENT_1__"),
    ("%2", "__PERCENT_2__"),
    ("%3", "__PERCENT_3__"),
    ("%4", "__PERCENT_4__"),
    ("%5", "__PERCENT_5__"),
    ("%6", "__PERCENT_6__"),
    ("%7", "__PERCENT_7__"),
    ("%8", "__PERCENT_8__"),
    ("%9", "__PERCENT_9__"),
    ("%L1", "__LOCALIZED_1__"),
    ("%L2", "__LOCALIZED_2__"),
    ("%L3", "__LOCALIZED_3__"),
    ("%L4", "__LOCALIZED_4__"),
    ("%L5", "__LOCALIZED_5__"),
    ("*.html", "__GLOB_HTML__"),
    ("*.txt", "__GLOB_TXT__"),
    ("*.png", "__GLOB_PNG__"),
    ("*.jpg", "__GLOB_JPG__"),
    ("*.jpeg", "__GLOB_JPEG__"),
    ("*.bmp", "__GLOB_BMP__"),
    ("*.gif", "__GLOB_GIF__"),
    ("*.tif", "__GLOB_TIF__"),
    ("*.tiff", "__GLOB_TIFF__"),
    ("*.webp", "__GLOB_WEBP__"),
    ("MergeQt", "__PROJECT_NAME__"),
    ("Qt Widgets", "__QT_WIDGETS__"),
)


@dataclass
class TranslationStats:
    updated: int = 0
    skipped: int = 0
    failed: int = 0


class TranslationError(RuntimeError):
    pass


class TranslatorBackend:
    def translate(self, text: str, source_lang: str, target_lang: str) -> str:
        raise NotImplementedError


class DeepTranslatorBackend(TranslatorBackend):
    def __init__(self, provider: str, proxies: dict[str, str] | None = None) -> None:
        try:
            from deep_translator import GoogleTranslator, MyMemoryTranslator
        except ImportError as exc:
            raise TranslationError(
                "Missing dependency: deep-translator. Install it with `python3 -m pip install --user deep-translator`."
            ) from exc

        self._proxies = proxies
        provider = provider.lower()
        if provider == "google":
            self._factory = GoogleTranslator
        elif provider == "mymemory":
            self._factory = MyMemoryTranslator
        else:
            raise TranslationError(f"Unsupported deep-translator provider: {provider}")

    def translate(self, text: str, source_lang: str, target_lang: str) -> str:
        translator = self._factory(source=source_lang, target=target_lang, proxies=self._proxies)
        translated = translator.translate(text)
        if translated is None:
            raise TranslationError("Translation backend returned no text.")
        return translated


@dataclass
class ProxySettings:
    source: str
    proxies: dict[str, str]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Traverse Qt .ts files and translate unfinished entries."
    )
    parser.add_argument(
        "--translations-dir",
        default="translations",
        help="Directory that contains Qt .ts files. Default: %(default)s",
    )
    parser.add_argument(
        "--files",
        nargs="*",
        default=[],
        help="Optional explicit .ts files. If omitted, the script scans the translations directory.",
    )
    parser.add_argument(
        "--provider",
        choices=("google", "mymemory"),
        default="google",
        help="Translation provider implemented via deep-translator. Default: %(default)s",
    )
    parser.add_argument(
        "--source-lang",
        default="en",
        help="Source language code for the translation backend. Default: %(default)s",
    )
    parser.add_argument(
        "--sleep-seconds",
        type=float,
        default=0.0,
        help="Optional delay between translated messages. Default: %(default)s",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print what would change without modifying any file.",
    )
    parser.add_argument(
        "--overwrite-finished",
        action="store_true",
        help="Also translate already-finished entries. By default only unfinished entries are touched.",
    )
    parser.add_argument(
        "--skip-languages",
        nargs="*",
        default=["en_US"],
        help="TS language tags to skip. Default: %(default)s",
    )
    parser.add_argument(
        "--proxy-mode",
        choices=("auto", "system", "env", "off", "custom"),
        default="auto",
        help="How to resolve HTTP(S) proxy settings. Default: %(default)s",
    )
    parser.add_argument(
        "--http-proxy",
        default="",
        help="Explicit HTTP proxy, for example http://127.0.0.1:7890",
    )
    parser.add_argument(
        "--https-proxy",
        default="",
        help="Explicit HTTPS proxy, for example http://127.0.0.1:7890",
    )
    parser.add_argument(
        "--test-connection",
        action="store_true",
        help="Test provider connectivity with the resolved proxy settings before translating.",
    )
    parser.add_argument(
        "--proxy-debug",
        action="store_true",
        help="Print the resolved proxy source and values.",
    )
    parser.add_argument(
        "--retries",
        type=int,
        default=3,
        help="How many times to retry a failed translation. Default: %(default)s",
    )
    parser.add_argument(
        "--retry-delay",
        type=float,
        default=1.5,
        help="Base delay in seconds before retrying a failed translation. Default: %(default)s",
    )
    return parser.parse_args()


def iter_ts_files(args: argparse.Namespace) -> list[Path]:
    if args.files:
        files = [Path(path) for path in args.files]
    else:
        files = sorted(Path(args.translations_dir).glob("*.ts"))

    missing = [path for path in files if not path.exists()]
    if missing:
        raise TranslationError("Missing translation files: " + ", ".join(str(path) for path in missing))

    return files


def build_proxy_dict(http_proxy: str, https_proxy: str) -> dict[str, str]:
    proxies: dict[str, str] = {}
    if http_proxy:
        proxies["http"] = http_proxy
    if https_proxy:
        proxies["https"] = https_proxy
    elif http_proxy:
        proxies["https"] = http_proxy
    return proxies


def parse_networksetup_proxy(output: str) -> str:
    enabled = False
    server = ""
    port = ""
    for raw_line in output.splitlines():
        line = raw_line.strip()
        if not line or ":" not in line:
            continue
        key, value = line.split(":", 1)
        key = key.strip()
        value = value.strip()
        if key == "Enabled":
            enabled = value.lower() == "yes"
        elif key == "Server":
            server = value
        elif key == "Port":
            port = value

    if not enabled or not server or not port:
        return ""
    return f"http://{server}:{port}"


def resolve_macos_system_proxy() -> dict[str, str]:
    try:
        services_output = subprocess.run(
            ["networksetup", "-listallnetworkservices"],
            capture_output=True,
            text=True,
            check=True,
        ).stdout
    except (FileNotFoundError, subprocess.CalledProcessError):
        return {}

    services = []
    for raw_line in services_output.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("An asterisk"):
            continue
        services.append(line.lstrip("*").strip())

    for service in services:
        try:
            web_output = subprocess.run(
                ["networksetup", "-getwebproxy", service],
                capture_output=True,
                text=True,
                check=True,
            ).stdout
            secure_output = subprocess.run(
                ["networksetup", "-getsecurewebproxy", service],
                capture_output=True,
                text=True,
                check=True,
            ).stdout
        except subprocess.CalledProcessError:
            continue

        proxies = build_proxy_dict(
            parse_networksetup_proxy(web_output),
            parse_networksetup_proxy(secure_output),
        )
        if proxies:
            return proxies

    return {}


def resolve_system_proxy() -> dict[str, str]:
    if sys.platform == "darwin":
        return resolve_macos_system_proxy()
    return {}


def resolve_proxy_settings(args: argparse.Namespace) -> ProxySettings:
    explicit = build_proxy_dict(args.http_proxy.strip(), args.https_proxy.strip())
    if args.proxy_mode == "off":
        return ProxySettings(source="off", proxies={})
    if args.proxy_mode == "custom":
        return ProxySettings(source="custom", proxies=explicit)
    if args.proxy_mode == "env":
        return ProxySettings(source="env", proxies=requests.utils.get_environ_proxies("https://translate.google.com"))
    if args.proxy_mode == "system":
        return ProxySettings(source="system", proxies=resolve_system_proxy())

    if explicit:
        return ProxySettings(source="custom", proxies=explicit)

    env_proxies = requests.utils.get_environ_proxies("https://translate.google.com")
    if env_proxies:
        return ProxySettings(source="env", proxies=env_proxies)

    system_proxies = resolve_system_proxy()
    if system_proxies:
        return ProxySettings(source="system", proxies=system_proxies)

    return ProxySettings(source="none", proxies={})


def provider_test_url(provider: str) -> str:
    if provider == "mymemory":
        return "https://api.mymemory.translated.net/get?q=test&langpair=en|zh-CN"
    return "https://translate.google.com"


def test_connection(provider: str, proxies: dict[str, str]) -> None:
    url = provider_test_url(provider)
    try:
        response = requests.get(url, timeout=15, proxies=proxies or None)
        response.raise_for_status()
    except requests.RequestException as exc:
        raise TranslationError(f"Connection test failed for {provider} via {url}: {exc}") from exc


def detect_target_language(root: ET.Element, ts_path: Path) -> str:
    language = root.attrib.get("language", "").strip()
    if not language:
        raise TranslationError(f"{ts_path}: missing TS language attribute.")

    target = LANGUAGE_CODE_MAP.get(language, language.replace("_", "-"))
    if not target:
        raise TranslationError(f"{ts_path}: unsupported TS language '{language}'.")
    return target


def detect_ts_language_tag(root: ET.Element, ts_path: Path) -> str:
    language = root.attrib.get("language", "").strip()
    if not language:
        raise TranslationError(f"{ts_path}: missing TS language attribute.")
    return language


def protect_text(text: str) -> tuple[str, bool]:
    has_accelerator = text.startswith("&") and len(text) > 1
    protected = text[1:] if has_accelerator else text

    for original, token in MASK_TOKENS:
        protected = protected.replace(original, token)

    return protected, has_accelerator


def restore_text(text: str, has_accelerator: bool) -> str:
    restored = text
    for original, token in MASK_TOKENS:
        restored = restored.replace(token, original)

    if has_accelerator and not restored.startswith("&"):
        restored = "&" + restored

    return restored


def translation_needs_update(translation: ET.Element, overwrite_finished: bool) -> bool:
    if overwrite_finished:
        return True
    return translation.attrib.get("type") == "unfinished"


def translate_message(
    backend: TranslatorBackend,
    source_text: str,
    source_lang: str,
    target_lang: str,
    retries: int,
    retry_delay: float,
) -> str:
    protected, has_accelerator = protect_text(source_text)
    last_error: Exception | None = None
    for attempt in range(max(1, retries)):
        try:
            translated = backend.translate(protected, source_lang, target_lang)
            if not translated:
                raise TranslationError("Translation backend returned an empty string.")
            return restore_text(translated, has_accelerator)
        except Exception as exc:  # noqa: BLE001
            last_error = exc
            if attempt + 1 >= max(1, retries):
                break
            time.sleep(retry_delay * (attempt + 1))

    raise TranslationError(str(last_error) if last_error is not None else "Translation failed.")


def process_ts_file(
    ts_path: Path,
    backend: TranslatorBackend,
    source_lang: str,
    sleep_seconds: float,
    overwrite_finished: bool,
    dry_run: bool,
    skip_languages: set[str],
    retries: int,
    retry_delay: float,
) -> TranslationStats:
    parser = ET.XMLParser(target=ET.TreeBuilder(insert_comments=True))
    tree = ET.parse(ts_path, parser=parser)
    root = tree.getroot()
    language_tag = detect_ts_language_tag(root, ts_path)
    if language_tag in skip_languages:
        return TranslationStats()
    target_lang = detect_target_language(root, ts_path)

    stats = TranslationStats()
    for message in root.iter("message"):
        source = message.find("source")
        translation = message.find("translation")
        if source is None or translation is None:
            continue

        source_text = source.text or ""
        if not source_text.strip():
            stats.skipped += 1
            continue
        if not translation_needs_update(translation, overwrite_finished):
            stats.skipped += 1
            continue

        try:
            translated_text = translate_message(
                backend,
                source_text,
                source_lang,
                target_lang,
                retries=retries,
                retry_delay=retry_delay,
            )
        except Exception as exc:  # noqa: BLE001
            stats.failed += 1
            print(f"[failed] {ts_path.name}: {source_text!r}: {exc}", file=sys.stderr)
            continue

        if translated_text == (translation.text or "") and "type" not in translation.attrib:
            stats.skipped += 1
            continue

        translation.text = translated_text
        translation.attrib.pop("type", None)
        stats.updated += 1

        if sleep_seconds > 0:
            time.sleep(sleep_seconds)

    if stats.updated > 0 and not dry_run:
        ET.indent(tree, space="    ")
        xml_bytes = ET.tostring(root, encoding="utf-8")
        document = b'<?xml version="1.0" encoding="utf-8"?>\n' + DOCTYPE.encode("utf-8") + b"\n" + xml_bytes
        ts_path.write_bytes(document)

    return stats


def main() -> int:
    args = parse_args()

    try:
        files = iter_ts_files(args)
        proxy_settings = resolve_proxy_settings(args)
        if args.proxy_debug:
            print(f"proxy source: {proxy_settings.source}")
            print(f"proxy values: {proxy_settings.proxies}")
        if args.test_connection:
            test_connection(args.provider, proxy_settings.proxies)
            print("connection test: ok")
        backend = DeepTranslatorBackend(args.provider, proxies=proxy_settings.proxies or None)
    except TranslationError as exc:
        print(exc, file=sys.stderr)
        return 1

    total = TranslationStats()
    for ts_path in files:
        stats = process_ts_file(
            ts_path=ts_path,
            backend=backend,
            source_lang=args.source_lang,
            sleep_seconds=args.sleep_seconds,
            overwrite_finished=args.overwrite_finished,
            dry_run=args.dry_run,
            skip_languages=set(args.skip_languages),
            retries=args.retries,
            retry_delay=args.retry_delay,
        )
        total.updated += stats.updated
        total.skipped += stats.skipped
        total.failed += stats.failed
        print(
            f"{ts_path.name}: updated={stats.updated}, skipped={stats.skipped}, failed={stats.failed}"
        )

    print(
        f"done: updated={total.updated}, skipped={total.skipped}, failed={total.failed}, dry_run={args.dry_run}"
    )
    return 0 if total.failed == 0 else 2


if __name__ == "__main__":
    raise SystemExit(main())
