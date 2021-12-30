@S Experiments
@Z (continued)

The [previous section](getting-started.html) introduced a series of experiments,
let's keep the momentup flowing.

##Experiment #2, Unaltered Project

The next experiment is to run winged-spider without forcing a rebuild
(no command line options). In this case Winged Spider 
should report that no html is generated (because the html outputs are
older than the markdown and template inputs). If you are familiar with the
classic 'make' utility in software development, this philosophy will be familiar,
Winged Spider seeks to avoid unnecessarily rebuilding the whole project, only
rebuilding those html files that need to be rebuilt because their inputs
changed.

For a further demonstation of this 'make' like behaviour, minimally modify
one of the input markdown files and run Winged Spider again. It should build the
corresponding html file (and only that one).

##Experiment #3, Force Rebuild by Deleting Output

Next temporarily rename the 'output' folder. When you run it
after doing that, Winged Spider will report a problem, it needs that 'output'
folder! So create a new (empty) 'output' folder.
Now Winged Spider will run, and it's a bit like a forced rebuild, all html files
are recreated. Additionally, Winged Spider reports in this case that the runtime
files needed by the templating system (.css and .js files in the 'template' directory,
more precisely all files in the 'template' directory other than the *.txt templates
themselves) are missing from the 'output' directory and it will copy them into
the 'output' directory for you.

If you use a browser to look at the recreated project after this latest rebuild you'll
notice a problem - there were image files in the original 'output' directory and
nothing has put these back - they exist outside Winged Spider's operational realm as
it were. Unlike some similar systems Winged Spider does not touch the 'output' directory
except for generating html files (and, as we have seen, bringing in builerplate .css and .js files
from the 'template' directory). This has the advantage that other types of assets can
sit forever in the 'output' directory, under manual control. The image files in the
example project are a good example.

( For the sake of completeness there is another exception to the 'nothing but generating
html files rule' above - Winged Spider supports one special type of media as an extension;
chess files. This reflects Winged Spider's origins as a tool built to maintain a chess
website. Winged Spider accepts .pgn [chess document] files as well as markdown files
for generating output html files and when it does that it adds a modified form of the
.pgn file to a predefined 'assets' subfolder of the 'output' folder. There's an example
in the 'Details' submenu. Sadly as I put this
tutorial together I noticed a small issue, as well as missing image files this experiment
reveals that a png folder in the new output directory is also absent, and it's needed by the
chess extension. Winged Spider should really copy it across from the 'template' folder
along with the .js and .css files. I've
made a note in [Quirks and Rough Edges](quirks-and-rough-edges.html) )

At this stage please solve the missing image file problem we've just identified in
by deleting the new output directory and renaming the temporary one back to 'output',
or simply start again from scratch.

##Experiment #4, The plan.txt file

Next create a new markdown file in the 'input' directory alongside Home.md, Getting Started.md
and the rest. Call it 'About.md' say and write a line or two of hello world type material
then save the file. Now when you run Winged Spider it will detect that something has
changed, the contents of the 'input' directory is no longer in sync with the plan.txt
file which records how the directory hierarchy should be presented to the user.
Winged Spider will include the new About.md file, and create and use its own generated-plan.txt
instead. Why doesn't it simply overwrite plan.txt? We can tell by comparing plan.txt and
generated-plan.txt (or by looking at the project in a browser).

This is plan.txt

<pre>
Home.md
Getting Started.md
Experiments.md
Details\
Details\Details.md
Details\Markdown Extensions.md
Details\Template System.md
Details\Chess.pgn
Details\Quirks and Rough Edges.md
</pre>

This is generated-plan.txt

<pre>
About.md
Home.md
Getting Started.md
Experiments.md
Details\
Details\Details.md
Details\Markdown Extensions.md
Details\Template System.md
Details\Chess.pgn
Details\Quirks and Rough Edges.md
</pre>

The problem is that Winged Spider is only a stupid computer program and it can't possibly
know the best place to put the new page, it just puts it in its alphabetical place, which
of course is no more effective than placing it randomly. It does its best to inform you of the
problem, and then it is up to you to fix it by modifiying plan.txt to place About.md in a more
sensible place;

Manually modify plan.txt as shown;

<pre>
Home.md
Getting Started.md
Experiments.md
About.md
Details\
Details\Details.md
Details\Markdown Extensions.md
Details\Template System.md
Details\Chess.pgn
Details\Quirks and Rough Edges.md
</pre>

Now when you run Winged Spider again it will note that the plan and the generated plan are
out of sync and it will use the differences to intelligently rebuild the Home, Folders,
Getting Started and About pages. These pages need to be rebuilt because their menu has
been reordered. The Details pages are unaffected since the Details menu is unchanged, so
they are not rebuilt.

Winged Spider only writes to plan.txt if it doesn't exist at all, but it creates a new
generated-plan.txt on every run. If you start a project from scratch then Winged Spider
will not only create generated-plan.txt it will copy it to plan.txt to give you a starting
point. As we have found, the main thing, perhaps the only thing, wrong with the auto generated
plan.txt will be that the pages will probably not be in the best or even a sensible order.

### Other possible experiments

Of course the experiments above are far from exhaustive.

Some other ideas.

- Try tweaking the header template in template-md.txt to better suit your preferences and your project. If you really
want to get into the weeds, research Yahoo's 'pure' CSS framework and adjust pure-customised.css.
There is some [reference information](details-template-system.html) in the Details submenu.

- Introduce a second folder in the hierarchy, or a subfolder to see how the menu system
works automatically for an arbitrarily deep hierarchy. Be prepared to manually adjust
plan.txt for best results.

- Add a line like this manually to plan.txt; <br><pre> Wikipedia -> https://wikipedia.org </pre> Hopefully this
is self explanatory - it's a way of having arbitrary links in the menu.

- Delete one of the markdown files. Winged Spider should report the problem, but go ahead and
build the project anyway. When you change the structure of the project, even in a small way,
the generated-plan.txt and the plan.txt files become out of sync (different). The
generated-plan.txt file reflects Winged Spider's reality, the plan.txt file reflects
your intentions. When they are out of sync it is best to either solve the underlying
problem (in this case a missing file) or update plan.txt to reflect reality.

- Add @S and @Z macros to modify the subtitle and sub-subtitle (refer to the [template system page](details-template-system.html) ).
The automatically generated sub-subtitle for this page is empty, but I've added @S and @Z macros
to the top of the markdown for this file to introduce "(continued)" as a sub-subheading. If you use these
particular macros, it's important to use them as a pair, you cannot mix and match with the autogenerated
subtitle and sub-subtitle. No such restrictions apply to any additional macros you might
introduce for the header and footer (like @T for title perhaps).