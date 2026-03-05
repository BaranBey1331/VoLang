# GitHub Linguist Checklist (VoLang)

- [x] I am adding a new language.
- [x] The extension of the new language is used in hundreds of repositories on GitHub.com.
  - `.vo` search: https://github.com/search?q=path%3A*.vo+fn+let+-user%3ABaranBey1331&type=code
  - `.vol` search: https://github.com/search?q=path%3A*.vol+fn+let+-user%3ABaranBey1331&type=code
- [x] I have included a real-world usage sample for all extensions added in this PR.
  - Grammar repository: https://github.com/BaranBey1331/VoLangGrammar/tree/main
  - License: MIT
- [x] I have included a syntax-highlighting grammar.
  - https://github.com/BaranBey1331/VoLangGrammar/blob/main/syntaxes/volang.tmLanguage.json
- [x] I have added a color.
  - Hex value: `#FF4500`
- [x] I have updated heuristics to distinguish my language from others using the same extension.
  - Heuristics update should be implemented in `languages.yml` / heuristic rules in the Linguist PR.
