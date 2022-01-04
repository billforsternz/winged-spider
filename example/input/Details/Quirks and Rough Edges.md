This a just a collection area for points that haven't arisen elsewhere, plus notes on things that
need improvement or otherwise might change (in as much as this can be anticipated!)

- Winged spider will always convert Home.md in the root of the 'input' folder to index.html in
the root of the output folder, hopefully as a convenience.

- Winged spider flattens the output directory structure, basically in order to avoid the difficulty
of referencing .css and .js files in nested parent folders.

- You can include html files in the 'input' folders and they will be copied to the 'output' without
extra processing. The user will not get the Winged Spider menu navigation when they alight on raw
html files like this, but they can still use the native browser features, in particular the back
button to get back into the menu system.

- BUG (update - fixed 2022.01.04). Winged spider fails to copy the png directory (or any directory actually) needed for the chess
extension from the 'template' directory to the 'output' directory automatically. For the moment it
must be copied manually, once, after which it will sit happily forever.

- It would possibly be better to accept user macros in the plan.txt file (indented say, in a list
directly after the corresponding markdown file) rather than in the first lines of the markdown
file itself. This is for further consideration/study.

- I want to use the '/' character instead of the '&#x5c;' character as a path separator everywhere. That will make
getting a Unix/Linux/Mac OS port going much easier, but I haven't quite got around to it yet.
