## Introduction
Winged Spider is intended to be a innovative (I hope), spectactularly easy to use static
website builder. The idea is that Winged Spider will take as input content organised
nicely in a well named directory hierarchy, and generate as output web ready content
with the same structure. The directory and file names from the input are reflected
directly in the menu structure presented to the end user for navigating the content.

This is the Winged Spider example project from the project's reference repository
at [Github](https://github.com/billforsternz/winged-spider). It does double
duty, as it's also the Winged Spider user documentation.

To make a website with Winged Spider, simply write some pages in markdown, and name them
sensibly. Then run Winged Spider with a pre-supplied or custom template file and get a
website that lets users navigate through all the content using a menu that that uses
the (sensible remember) filenames directly.

As your project grows it will probably make sense to introduce a folder hierarchy. Again,
use sensible folder names because those folder names are now going to
be used for submenus, reflecting the folder hierarchy in the resulting website. This is
accomplished automatically with no work from you. If you don't find this idea neat or
attractive it's probably best to forget Winged Spider and move on now!

The first pre-supplied template is based on "pure", a CSS framework from Yahoo.
Pure is not being maintained, but it works well for simple and medium sized projects and
sometimes not being maintained is a feature not a bug (because the sands aren't shifting
under you, you're using something that's finished and works).

This is a simple example website built with Winged Spider, and also an explanation
of Winged Spider. It should be available either as a straightforward directory of text
files called example (or an example.zip file containing such a directory) with any
Winged Spider distribution.

The next section, [Getting Started](getting-started.html), is intended to provide a quick
start. Here's a tip for the really impatient though - You might be the sort of person
who can get started even quicker like this; Grab a copy of the example project. Delete the
plan.txt file and the contents of the 'input' and 'output' folders. Create a single markdown 
file called Home.md in the 'input' folder and put Hello World or similar into it. Run Winged
Spider from the command line. Job done.
