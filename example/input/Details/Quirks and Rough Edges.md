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

- (update - fixed before V1.00) I want to use the '/' character instead of the '&#x5c;' character as a path separator everywhere. That will make
getting a Unix/Linux/Mac OS port going much easier, but I haven't quite got around to it yet.

- Winged Spider creates html files in Unix end of line text format ('&#x5c;n') not Windows ('&#x5c;r&#x5c;n') text format, even on Windows.
Conversely it creates PGN asset files in Windows text format even on Unix. The reason is that Unix text format is really the native
format for html, and PGN is a consumer format most commonly used on Windows systems. As of V1.00 there is an outstanding problem that the PGN asset files are created in an unsatisfactory mixed line
endings format if the input PGN file is in Unix format. Sorry, to be fixed.
