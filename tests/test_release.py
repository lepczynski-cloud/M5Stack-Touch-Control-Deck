#!/usr/bin/env python3
"""Offline regression checks for the firmware/web integration (stdlib only).

Run: python3 -m unittest discover -s tests -v
These checks do not replace compiling on PlatformIO or testing a real Mac.
"""
from __future__ import annotations

import json
import re
import shutil
import subprocess
import unittest
from html.parser import HTMLParser
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class WebsiteParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids = []
        self.translation_keys = set()
        self.manifests = set()
        self.local_refs = set()

    def handle_starttag(self, tag, attrs):
        attr = dict(attrs)
        if 'id' in attr:
            self.ids.append(attr['id'])
        if 'data-i18n' in attr:
            self.translation_keys.add(attr['data-i18n'])
        if tag == 'esp-web-install-button':
            self.manifests.add(attr.get('manifest'))
        for name in ('href', 'src'):
            value = attr.get(name, '')
            if value.startswith('./') and not value.startswith('./firmware/'):
                self.local_refs.add(value)


class ReleaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        cls.js = (ROOT / 'web/assets/site.js').read_text(encoding='utf-8')
        cls.parser = WebsiteParser()
        cls.parser.feed(cls.html)

    def test_project_source_version_matches_two_manifests(self):
        header = (ROOT / 'include/UserConfig.h').read_text(encoding='utf-8')
        match = re.search(r'FIRMWARE_VERSION\[\]\s*=\s*"(\d+\.\d+\.\d+)"', header)
        self.assertIsNotNone(match)
        expected = match.group(1)
        for manifest, bin_name, offset in (
            ('manifest.json', 'm5stack-touch-control-deck.factory.bin', 0),
            ('manifest-update.json', 'm5stack-touch-control-deck.update.bin', 65536),
        ):
            with self.subTest(manifest=manifest):
                obj = json.loads((ROOT / 'web/firmware' / manifest).read_text(encoding='utf-8'))
                self.assertEqual(obj['version'], expected)
                self.assertTrue(obj['new_install_prompt_erase'])
                self.assertEqual(obj['builds'][0]['chipFamily'], 'ESP32-S3')
                self.assertEqual(obj['builds'][0]['parts'], [{'path': bin_name, 'offset': offset}])
        self.assertEqual(self.parser.manifests, {
            './firmware/manifest.json', './firmware/manifest-update.json',
        })

    def test_site_links_and_translations(self):
        self.assertEqual(len(self.parser.ids), len(set(self.parser.ids)), 'Duplicate HTML ids')
        en = self.js.split('  en: {', 1)[1].split('\n  },\n  pl: {', 1)[0]
        pl = self.js.split('  pl: {', 1)[1].split('\n  }\n};', 1)[0]
        keys = lambda text: set(re.findall(r'(?:^|,)\s*([A-Za-z]\w*):"', text, re.MULTILINE))
        en_keys, pl_keys = keys(en), keys(pl)
        self.assertTrue(en_keys)
        self.assertEqual(en_keys, pl_keys, 'Missing PL/EN translation keys')
        self.assertEqual(self.parser.translation_keys - en_keys, set(), 'HTML uses undefined translation keys')
        for ref in self.parser.local_refs:
            self.assertTrue((ROOT / 'web' / ref).is_file() or
                            ref == './assets/m5stack-touch-control-deck.jpg',
                            f'Missing static asset: {ref}')
        self.assertIn('prefers-reduced-motion', (ROOT / 'web/assets/site.css').read_text(encoding='utf-8'))
        self.assertIn('requestAnimationFrame', self.js)
        self.assertIn('youtube-nocookie.com/embed', self.js)
        self.assertIn('https://unpkg.com/esp-web-tools@10/', self.html)

    def test_mac_shortcuts_are_configured_and_persisted(self):
        ble = (ROOT / 'src/BleController.cpp').read_text(encoding='utf-8')
        portal = (ROOT / 'src/NetworkPortal.cpp').read_text(encoding='utf-8')
        manager = (ROOT / 'src/ConfigManager.cpp').read_text(encoding='utf-8')
        main = (ROOT / 'src/main.cpp').read_text(encoding='utf-8')
        self.assertIn('void BleController::setMacUrlMode(MacUrlMode mode)', ble)
        self.assertIn("static_cast<uint8_t>('1' + tileIndex)", ble)
        self.assertIn('tileIndex >= 6', ble)
        self.assertIn('mac_url_mode', portal)
        self.assertIn('"mac_mode"', manager)
        self.assertIn('bleController.setMacUrlMode', main)
        self.assertIn('MACOS_FAST_SHORTCUTS.md', (ROOT / 'README.md').read_text(encoding='utf-8'))

    def test_publish_pipeline_remains_unchanged(self):
        build = (ROOT / 'tools/build_web_firmware.sh').read_text(encoding='utf-8')
        pack = (ROOT / 'tools/package_web_firmware.py').read_text(encoding='utf-8')
        self.assertIn('platformio run -e cores3_se', build)
        self.assertIn('tools/package_web_firmware.py', build)
        self.assertIn('cp images/m5stack-touch-control-deck.jpg web/assets/', build)
        self.assertIn('"0x10000"', pack)
        self.assertIn('"0x0"', pack)
        self.assertIn('shutil.copyfile(app, UPDATE_OUTPUT)', pack)
        self.assertIn('directory = "./web"', (ROOT / 'wrangler.toml').read_text(encoding='utf-8'))

    def test_web_image_is_light_and_firmware_is_not_stored_in_git(self):
        photo = (ROOT / 'images/m5stack-touch-control-deck.jpg')
        self.assertLess(photo.stat().st_size, 500_000, 'Use optimized source photo')
        self.assertTrue(photo.read_bytes().startswith(b'\xff\xd8'))
        ignored = (ROOT / '.gitignore').read_text(encoding='utf-8')
        self.assertIn('web/assets/m5stack-touch-control-deck.jpg', ignored)
        self.assertIn('web/firmware/*.factory.bin', ignored)
        self.assertIn('web/firmware/*.update.bin', ignored)

    def test_javascript_parses_when_node_available(self):
        node = shutil.which('node')
        if not node:
            self.skipTest('Node.js not available')
        subprocess.run([node, '--check', str(ROOT / 'web/assets/site.js')], check=True)


if __name__ == '__main__':
    unittest.main()
