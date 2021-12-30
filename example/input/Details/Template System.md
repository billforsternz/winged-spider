#Template System

The template system that comes with this example is sufficient to get started with
Winged Spider, this is supplementary information that will hopefully be of help if
the user wants or needs to modify it or create a new template with different third
party underpinnings.

The file template-md.txt within the 'template' directory serves as the template for
markdown to html generation. The file is divided into sections separated by a single
blank line. There are 9 sections header, footer, solo, panel, single, pair, triple,
1of3, 2of3. Each section is html text, with a simple macro system for parameterisation.
The first line of each section is not part of the section proper, it serves to
identify the section, and comprises an '@' character followed by the section name,
for example @header.

The header and footer sections provide whatever boilerplate is required at the start
and end of the html file, and then introduce and close the content section that will
correspond to the markdown provided by the user.

Header and footer alone would actually be sufficient for a truly minimalist system,
but Winged Spider chooses to also support some presentation features over and above
base markdown, and the other seven sections are there to enable them. The solo and
panel sections define parameterised html corresponding to two special paragraph types.
Solo is a heading, plus a picture plus some text that flows around the picture.
Panel is a highlighted purple panel. The other five sections define grid layouts.
More information on the solo, panel and the grid features can be found
[here](details-markdown-extensions.html).

Parameterised macros are an important part of the Winged Spider template system
but the user will typically not need to use them at all unless they wish to make
significant changes to the existing Yahoo 'Pure' template, or create a brand new
template system.

In all cases macros are represented by a single @ character followed by a single
ASCII letter (it's a simple system!). As an example look at the 'solo' template from the template-md.txt
file in the template directory;

<pre>
&lt;div&gt;
&lt;h2&gt;@H&lt;/h2&gt;
&lt;p&gt;
&lt;img style=&quot;float:left;width:50%;margin-right:1.5em;margin-bottom:1em&quot; src=&quot;@F&quot; alt=&quot;@A&quot;&gt;
&lt;/p&gt;
@T
&lt;/div&gt;
&lt;div style=&quot;clear:both;&quot;&gt;&lt;/div&gt;
</pre>

In this @H, @F, @A and @T are parameters which are replaced by the (H)eader text, (F)ilename,
(A)lt text and multi paragraph html paragraph (T)ext respectively from the solo instantiation. 
The user defines these things by writing markdown like this;

<pre>
<br>
##This is an Example Solo
![Horses](images/horses.jpg)Caption currently unused by template
For the website that Winged Spider was first used on, small stories were the bread and
butter components. A story was basically a heading, a picture and some text. Sometimes
the story didn't have a picture, so a distinction was made between stories with an image
(a 'solo') and stories without (a 'snippet').
<br>
Multiple paragraphs of text are possible, like this. To make a solo, simply arrange
some markdown with the heading, the image and the associated text in the exact pattern
shown below;
<br>
</pre>

So for example @F is 'This is an Example Solo', @A is 'Horses', @T is the paragraphs of text, after
it has been converted from markdown to html. The Caption part of the markdown image
syntax is represented by @C and is not currently used by the template, a missed
opportunity actually.

If you take a look at template-md.txt, you will see that the largest template is the
'header' template. It is basically a chunk of boilerplate html that starts each page
before the actual content of the page (as represented by the markdown) starts. As
such the header, together with the much smaller footer, is responsible for pulling
in the CSS and Javascript files that will be used at
to provide styling and menu navigation. You will almost certainly want to make
at least some modifications to this template to reference your organisation and project.
You won't initially want to change the CSS and Javascript invocation, but it might be
useful to understand the macros that the header template uses anyway.

Winged Spider looks for the macro @M in the header template to find the start of
a list of menu items. There should then be two lines, one representing a normal
menu item, and a second representing the current highlighted menu item. In both
cases the macros @1 and @2 are used to indicate the link destination
of the menu item and the text of the menu item respectively. Winged Spider completely automates
this menu building process, the user (again, except for someone creating a template)
does not need to care about it at all.

The other two macros that appear in the default Winged Spider header template and
which Winged Spider has built in support for are @S and @Z which stand for Subtitle
and Sub-Subtitle respectively.

Winged Spider calculates a default Subtitle Sub-subtitle for you using the names of the
folder hierarchy, but you *may* want to override these. Winged spider looks for user
macro definitions in the first lines of each input markdown file it processes. The
syntax is simply;

<pre>
@S User Subtitle
@Z User Sub-subtitle
</pre>

Put any such definitions only on the very first lines of the markdown file, with no indent
and follow them with a blank line. You can add additional macros for your modifications to
the header (and/or footer) templates if required. For example you might want to parameterise
the title, in which case @T is a natural fit. Just avoid clashes with the built in support
for @M, @S and @Z.

Note that if either a User Title or User Subtitle are defined, Winged Spider discards its
auto generated title AND subtitle, so if you define only one the other will be blank, this
is desirable behaviour, the definitions work best as a pair and you won't want to mix and
match them.
