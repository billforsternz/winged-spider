# Winged Spider, a static website builder

Winged Spider is a spectactularly easy to use, very fast static
website builder. Winged Spider takes as input content organised
nicely in a well named directory hierarchy, and generates as output web ready content
with the same structure. The directory and file names from the input are used
to create the website's menu structure, which means well named files and
folders, and a coherent hierarchy automatically generates a very usable website.

So to make a website, write your pages in markdown, name them sensibly and organise
them in hierarchical folders. Then run Winged Spider with a pre-supplied or custom
template file and get a website that lets users navigate through all the content in an
intuitive way, using menus that directly reflect the structure defined by the folder
hierarchy.

The first pre-supplied template is based on "pure", a CSS framework from Yahoo.
I had good results with it with manually generated pages, and am now having
equally good results using Winged Spider to make my websites instead.

Pure is not being maintained, but it
works well for simple projects and sometimes not being maintained is a feature not a
bug (because the sands aren't shifting under you, you're using something that's finished
and works).

# Status

Winged Spider V1.00 is now released. It has been used in production for
an important, medium sized project (the New Zealand Chess Federation website) for several
weeks even before V1.00.

In preparation for V1.00 I have created comprehensive user documentation and a full
example (checked into the repo in the 'example' directory). In fact I fed two birds
with one scone because the example is the documentation! I have put
the [rendered example here](https://triplehappy.com/winged-spider-example/output/index.html)
and it's also now available directly via [Github pages](https://billforsternz.github.io/winged-spider/example/output/index.html).

# Chess

Winged Spider was created for a chess website (New Zealand Chess Federation website, and has
built in support for interactive chess presentation. For more details see the
[example project](https://triplehappy.com/winged-spider-example/output/details-chess.html)

# Building Winged Spider

Winged Spider has been tested on Windows, Linux and Mac. Simple project and solution files
for Visual Studio C++ (2017) on Windows are provided. Windows and Mac executables are in the release
.zip. I have tried to make it easy to build yourself on any platform. All that is needed is
a C++17 capable compiler and linker. For simplicity's sake all the source files are in one
directory (src), and even the third party MD4C c files have been renamed as cpp, with a few
minor adjustments to make them correct C++ programs (mainly casting void pointers from malloc
and realloc to typed pointers).

For example, the command line to build the Mac Intel executable was;

    g++ -O3 -std=c++17 -o ws-mac-x64_86 -mmacosx-version-min=10.15 -arch x64_86 src/*.cpp
    
Note that this means the Mac version requires macOS 10.15 (Catalina) or later. This is a consequence
of using the C++17 \<filesystem\> facilites to navigate the directory structure. Support for this has
only been provided for recent macOS versions. There are much older well known Unix APIs for navigating
directories, and if necessary, with a bit of work, it would be possible to retrofit them into
Winged Spider.

# Acknowledgements

Thanks to Martin Mitáš (mity on Github) for the MD4C markdown parser.

# The name means what?

The name Winged Spider means nothing - it's just a cool name, and I bought wingedspider.com
a few years back.

Bill Forster <billforsternz@gmail.com>
