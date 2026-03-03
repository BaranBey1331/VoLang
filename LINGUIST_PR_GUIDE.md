VoLang - GitHub Linguist PR Checklist
When we are ready to officially add VoLang to GitHub, we will copy this checklist into the Pull Request description on the github-linguist/linguist repository.
 * [ ] I am adding a new language.
 * [ ] The extension of the new language is used in hundreds of repositories on GitHub.com.
   * Note: We need to ensure community adoption before submitting.
 * [ ] Search results for each extension:
   * To prove usage and filter out our own repository (-user:YOUR_GITHUB_NAME):
   * .vo search: https://www.google.com/search?q=https://github.com/search%3Fq%3Dpath%253A*.vo%2Bfn%2Blet%2B-user%253AYOUR_GITHUB_NAME%26type%3Dcode
   * .vol search: https://www.google.com/search?q=https://github.com/search%3Fq%3Dpath%253A*.vol%2Bfn%2Blet%2B-user%253AYOUR_GITHUB_NAME%26type%3Dcode
 * [ ] I have included a real-world usage sample for all extensions added in this PR:
   * We will add sample .vo scripts (like math.vo) into the samples/VoLang/ directory in the Linguist repo.
 * [ ] Grammar repo:
   * https://www.google.com/search?q=https://github.com/YOUR_GITHUB_NAME/VoLangGrammar
   * License: MIT License
 * [ ] I have included a syntax highlighting grammar:
   * https://www.google.com/search?q=https://github.com/YOUR_GITHUB_NAME/VoLangGrammar/blob/main/syntaxes/volang.tmLanguage.json
 * [ ] I have added a color:
   * Hex value: #FF4500 (VoLang Orange/Red)
 * [ ] I have updated the heuristics to distinguish my language from others using the same extension.
   * Since .vo might be used by obscure 3D object files (like ancient VideoScape files), we will add a heuristic rule in heuristics.yml to identify VoLang by looking for keywords like fn , let , and print(.
