import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from check_translations import (EXPECTED, anchors, check_pair, check_scope, digest,
                                numeric_difference, strip_nav)
from check_publication import author_manuscript


class TranslationChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.ko, self.en = 'README.md', 'README.en.md'
        self.nav = ('<!-- language-nav:start -->\n'
                    '[한국어](README.md) | [English](README.en.md)\n'
                    '<!-- language-nav:end -->\n\n')
        self.source = '# 소개\n\n' + self.nav + ('## 설치\n\n'
            '게임 버전 16. [같은 문서](README.md#설치)와 ![그림](image.png).\n\n'
            '| 기능 | 값 |\n|---|---|\n| 속도 | 25 |\n\n'
            '```sh\npython3 tools/check.py\n```\n\n`GameSession`\n')
        self.english = '# Overview\n\n' + self.nav + ('<a id="설치"></a>\n\n'
            '## Install\n\nGame version 16. [This document](README.en.md#설치) '
            'and ![Image](image.png). The regular game uses the same shared core '
            'as the cheat application. Select the required destination in the workspace '
            'and run the build with the development team configured for your device.\n\n'
            '| Feature | Value |\n|---|---|\n| Speed | 25 |\n\n'
            '```sh\npython3 tools/check.py\n```\n\n`GameSession`\n')
        (self.root / 'image.png').write_bytes(b'fixture')
        self.write()
        self.record = self.record_for(self.source, self.english)

    def record_for(self, source, english):
        return {'source_sha256': digest(source), 'english_sha256': digest(english),
                'reviewed_numeric_difference': numeric_difference(source, english)}

    def write(self):
        (self.root / self.ko).write_text(self.source)
        (self.root / self.en).write_text(self.english)

    def errors(self):
        self.write()
        return check_pair(self.root, self.ko, self.en, self.record)

    def test_author_hash_allows_only_the_fixed_series_navigation(self):
        path = 'docs/RESTORATION.md'
        intro = '# Author statement\n\n'
        body = 'Original author paragraph.\n'
        links = (
            '<!-- series-statement-nav:start -->\n'
            '[MachDefense](https://github.com/gelb2/MachDefense/blob/main/docs/RESTORATION.md) '
            '([English](https://github.com/gelb2/MachDefense/blob/main/docs/en/RESTORATION.md)) · '
            '[HeavyMach 2](https://github.com/gelb2/HeavyMach2/blob/main/docs/RESTORATION.md) '
            '([English](https://github.com/gelb2/HeavyMach2/blob/main/docs/en/heavymach2-restoration.md))\n'
            '<!-- series-statement-nav:end -->\n\n'
        )
        self.assertEqual(author_manuscript(path, intro + links + body), intro + body)
        with_inserted_prose = links.replace('<!-- series-statement-nav:end -->', 'Another paragraph.\n<!-- series-statement-nav:end -->')
        wrong_destination = links.replace('gelb2/MachDefense', 'gelb2/HeavyMach', 1)
        for invalid in (with_inserted_prose, wrong_destination):
            self.assertNotEqual(author_manuscript(path, intro + invalid + body), intro + body)
        self.assertNotEqual(author_manuscript(path, intro + links + 'Changed text.\n'), intro + body)

    def test_valid_pair_and_shared_image(self):
        self.assertEqual(self.errors(), [])

    def test_source_change_requires_review(self):
        self.source += '\n새 요구를 추가했다.\n'
        self.assertTrue(any('source changed' in x for x in self.errors()))

    def test_unreviewed_english_change(self):
        self.english += '\nChanged meaning.\n'
        self.assertTrue(any('translation changed' in x for x in self.errors()))

    def test_missing_translation(self):
        (self.root / self.en).unlink()
        self.assertTrue(check_pair(self.root, self.ko, self.en, self.record))

    def test_wrong_language_destination(self):
        self.english = self.english.replace('[한국어](README.md)', '[한국어](README.en.md)')
        self.assertTrue(any('wrong language destinations' in x for x in self.errors()))

    def test_missing_section_anchor(self):
        self.english = self.english.replace('id="설치"', 'id="다른절"')
        self.assertTrue(any('missing anchor' in x for x in self.errors()))

    def test_missing_image(self):
        (self.root / 'image.png').unlink()
        self.assertTrue(any('missing link' in x for x in self.errors()))

    def test_link_to_korean_instead_of_english(self):
        self.english = self.english.replace('[This document](README.en.md', '[This document](README.md')
        self.assertTrue(any('link targets differ' in x for x in self.errors()))

    def test_changed_command_even_with_updated_hash(self):
        self.english = self.english.replace('python3 tools/check.py', 'python3 tools/delete.py')
        self.record = self.record_for(self.source, self.english)
        self.assertTrue(any('code block differs' in x for x in self.errors()))

    def test_changed_identifier_even_with_updated_hash(self):
        self.english = self.english.replace('`GameSession`', '`GameSave`')
        self.record = self.record_for(self.source, self.english)
        self.assertTrue(any('identifiers differ' in x for x in self.errors()))

    def test_changed_number_needs_review(self):
        self.english = self.english.replace('| 25 |', '| 10 |')
        self.assertTrue(any('numeric differences' in x for x in self.errors()))

    def test_missing_table_row(self):
        self.english = self.english.replace('| Speed | 25 |\n', '')
        self.assertTrue(any('table structure' in x for x in self.errors()))

    def test_scope_rejects_internal_document_and_duplicates(self):
        scope = {'documents': [{'korean': a, 'english': b} for a, b in EXPECTED.items()]}
        self.assertEqual(check_scope(scope), [])
        for entry in ({'korean': 'AGENTS.md', 'english': 'AGENTS.en.md'},
                      {'korean': 'docs/canonical.md', 'english': 'docs/en/canonical.md'},
                      scope['documents'][0]):
            bad = copy.deepcopy(scope)
            bad['documents'].append(entry)
            self.assertTrue(check_scope(bad))

    def test_only_navigation_removed_from_author_text(self):
        original = '# 제목\n\n저자가 쓴 문장.  \n다음 줄.\n'
        with_nav = original.replace('\n\n', '\n\n' + self.nav, 1)
        self.assertEqual(strip_nav(with_nav), original)
        self.assertEqual(digest(with_nav), digest(original))
        self.assertNotEqual(digest(with_nav.replace('쓴 문장', '쓰지 않은 문장')), digest(original))

    def test_repeated_headings_and_explicit_anchors(self):
        self.assertEqual(anchors('# Title\n## A\n## A\n<a id="한국어"></a>\n'),
                         {'title', 'a', 'a-1', '한국어'})

    def test_link_cannot_escape_repository(self):
        self.english += '\n[Outside](../outside.md)\n'
        self.assertTrue(any('escapes repository' in x for x in self.errors()))


if __name__ == '__main__':
    unittest.main()
