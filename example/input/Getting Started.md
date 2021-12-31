##The Input Folder
The example project, just like any Winged Spider project is structured as a series of
standard subdirectories (also known as folders). The 'input' folder contains all of
the content to be rendered into a website. The defining idea of Winged Spider
is that the user will get to navigate around in the ultimate rendered website using
menus defined by the names given to the content files and folders.

## The Output Folder

Alongside the 'input' folder are two other folders required for every Winged Spider
project. Perhaps you've already guessed that one of them is 'output', which is where
Winged Spider puts the completed html (and css, and javascript) of the completed
project. Winged Spider is a static website generator, its responsibilities end with
the generation of the completed 'output'. To actually deploy the output onto a live
website is a separate topic beyond the scope of Winged Spider itself. For my own
projects I tend to use a program called Filezilla (available for Windows for example
on ninite.com) to simply copy the contents of the output folder to the html directory
of an Apache web server. You may have other arrangements.

## The Template Folder
The other fundamental folder is 'template'. Unsurprisingly I suppose this is where
Winged Spider looks for the template that tells it how to render markdown into
the final website. It would be terribly inflexible to have this information baked
permanently into the Winged Spider executable, some kind of template feature is
essential. The Winged Spider templating system is very simple. Initially there is
just one template, using a simple CSS framework from Yahoo called Pure (you can
Google it). It would be nice if someday there are other Winged Spider templates,
you can never tell how these things are going to go. For the moment Winged Spider
and its single template is sufficient for live operation of the significant medium
sized production website it was created for, so I am confident it is fit for purpose.

## It's a Command Line Tool
Winged Spider is a command line tool, a single executable program that
runs with minimal command line arguments. The initial target for Winged
Spider is Windows, and the program there is called winged-spider.exe. I
plan to test it on Linux and Mac OS as well. If you need to
compile it from source - the key thing to know is that it's just a matter of compiling
and linking the C++ files (as found in the Github repository), in the
simplest possible way, there's no need for external dependencies or special
options. Run the Winged Spider executable in your project directory, the
directory with 'input', 'output' and 'template' directories. Run it
with a '-?' argument to get a list of command line options.

## Learn by doing
To get started with Winged Spider, the best idea is to make a copy of the
Example project that comes with the Winged Spider distribution and start
experimenting with it. Once you've got a feel for Winged Spider that way,
you should be better able to judge whether it is suitable for whatever
project you have in mind for it. If you judge that Winged Spider is
suitable, you might decide to start that project by modifying a copy of
the example project, or by starting afresh, it's a matter of personal
taste.

For best results make sure you can quickly grab a fresh copy of the
unaltered example project, that way you can restart the experimentation
from a known starting point easily, at any stage.

## Experiment 1) Rebuilding the Example Project
To start off, rebuild the example project. To do that run the winged spider
executable in the project directory, with the -f flag. The -f flag indicates
'force a complete rebuild'. Winged Spider should provide an informative report
on its operations that will help if for some reason you need to troubleshoot
this basic operation. Assuming all goes well you should see newly rebuilt
html files in the 'output' folder and you should be able to browse them with
any browser.

The [next section](experiments.html) has a series of further experiments where
we can continue to learn by doing.
