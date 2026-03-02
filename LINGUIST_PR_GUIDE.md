GitHub Linguist Submission Guide
This document outlines the steps required to officially add VoLang to GitHub's language statistics (via the github-linguist/linguist repository).
📋 Official Requirements
According to the Linguist CONTRIBUTING.md, a language must meet these strict criteria before a PR can be accepted:
 * In-the-wild Usage: VoLang must be used in hundreds of unique, non-fork repositories on GitHub.
 * TextMate Grammar: We must provide a valid TextMate grammar (.tmLanguage.json or .cson) for syntax highlighting.
 * Open Source License: The grammar repository and VoLang itself must have an OSI-approved license (e.g., MIT).
📝 Steps to Submit
Once we reach the usage threshold, we will fork github-linguist/linguist and make the following changes:
 * Add our TextMate grammar repository as a submodule in vendor/grammars/.
 * Update languages.yml with the following block:
 * 
VoLang:
  type: programming
  color: "#FF4500"
  extensions:
    - ".vo"
    - ".vol"
  tm_scope: source.volang
  ace_mode: text
  language_id: <Next_Available_ID>

 * Add sample .vo files to the samples/VoLang/ directory in their repository.
 * Run script/add-grammar and script/test to ensure compliance before opening the Pull Request.
