#XML 2 C++

###What is it ?

If you have to deal with XML documents to initialize your C++ data, don't have
access for any reason to an XML schema and/or don't want to use a code generator
to create the data binding code, this library is for you.

This library helps you describe directly in C++ an XML document structure with
a type associated to each element. It then offers automated data generation
upon parsing an XML document (currently using
[expat](http://expat.sourceforge.net/) as a backend).

This is C++. The global coherence of the document structure and data binding is
guaranteed at compilation-time.

###Why not use a code generator based on an XML schema document ?

Sometimes you just don't have access to an XML schema. Sometimes you already
have complex data structures, and don't want to add the bloat of third-party
generated classes that you will need to convert into your own data structures,
or write adapters for.

Sometimes both.

For example in my line of work (engineering for academic research),
under-specification, especially of data formats, is the golden rule. It's all
nice and funky until someone has to maintain the code and make it evolve as
needed. So there are typically recurrent issues :

* The format specification is very sloppy/incomplete
* The requirements (contents, structure...) changes very often
* The data structures we use/need may themselves change often

This is where this library comes to help.

* You can (and must) directly describe how to bind XML elements to parts of *YOUR* data structures.
* You can (and must) directly describe the structure of the XML elements directly in the C++ code, where it matters the most.
* As I said above, the global coherence of the document (unicity of the element declaration/structure, typing consistency...) is checked at compile-time. No runtime surprise !
* There is no additional step in the compilation process to generate a source file implementing the XML-bound classes.

##Get started

1. Clone the git repository. All the necessary files are in the /include/ directory.
```
$ git clone https://github.com/bl0b/x2c.git && cd x2c
```
2. Optionally, run the tests, just to make sure that it works, and your compiler does handle C++11.
```
$ make    # this just compiles and runs the tests. The library is header files only.
```
3. Install the headers
```
$ make install   # if you skipped the 2nd step, it will perform it for you.
```

##Usage

```C++
#include <x2c/XML.h>
```

The API comes with macros to make your life easier. I'll demonstrate how to use X2C with and without them.

Easy version (using C preprocessor macros, beware)

```C++
struct UselessType {
    std::string wibble;
    int wobble;
};
struct MyUselessMainType {
    UselessType foo;
};

DTD_START(dtd_name, root_element, MyUselessMainType)
    /* The following line declares another element */
    ELEMENT(another_element, UselessType);
    /* Definition of the structure of the root element */
    /* It merely contains one <another_element> */
    root_element = E(another_element, &MyUselessMainType::foo);
    /* Structure of another_element */
    /* It has two attributes, which can be found in any order, and no
       contents. */
    another_element = (A("wibble", &UselessType::wibble),
                       A("wobble", &UselessType::wobble));
DTD_END(dtd_name);

std::ifstream in("path/to/a/document.XML");
MyUselessType* test = dtd_name.parse(in); /* can take any std::istream */
delete test;
```

Hard version

```C++
using namespace x2c;   /* This is for the sake of the example.
                          It is bad practice to import a whole namespace like that.
                          Please don't do this in real life. */

struct UselessType {
    std::string wibble;
    int wobble;
};
struct MyUselessMainType {
    UselessType foo;
};

Element<MyUselessMainType> root_element("root_element");
Element<UselessType> another_element("another_element");
root_element = E(another_element, &MyUselessMainType::foo);
another_element = (A("wibble", &UselessType::wibble),
                   A("wobble", &UselessType::wobble));

std::ifstream in("path/to/a/document.XML");
MyUselessMainType* test = XMLReader().parse_from(root_element_name, in);
delete test;
```
##What's happening there?

In the example, we assume two pre-existing data structures, UselessType and
MyUselessMainType.
We also assume an XML document like:

    <root_element>
        <another_element wibble="I can foo baz cheezbugah" wobble="6106"/>
    </root_element>

which we want to parse and instantiate a MyUselessMainType from.

Then we describe using X2C the binding :
- root_element maps to an instance of MyUselessMainType; it contains :
    - another_element maps to an instance of UselessType; it contains :
        - an attribute wibble the value of which maps to the field wibble,
        - an attribute wobble the value of which maps to the field wobble.

There are three operators to define the structure of an Element<> :

- | (pipe) to indicate alternatives (OR),
- & (ampersand) to indicate a sequence (FOLLOWED BY),
- , (comma) to indicate a pool or unordered sequence (AS WELL AS).

In this small example, we only used the comma to indicate that attributes can
appear in any order, we don't care, there's no side effect, and we take into
account the fact that the XML can be generated by various things (or humans)
that can be more or less sloppy. Any binding can also be made optional. A list
of alternatives, a sequence, or a pool CAN NOT be made optional.

Now, a word about binding to a field. There can be many ways you want the XML
test transformed into C++ data, depending on the types of both the target field
and the bound element.

In the following, `T` and `U` designate any type, and `C` designates a C++ container class.

- Binding `Element<T>` to a field of type `T`: direct binding. 
- Binding `Element<T>` to a field of type `T*`: the field receives a `new T()` (note the default constructor here, this is a hard requirement) which will be edited by the subsequent inner XML.
- Binding `Element<C::value_type>` to a field of type `C`: a `new value_type()` is created and will be edited by subsequent inner XML. When the element completes, the data is either `push_back()`'ed or `insert()`'ed into the container, in this order of preference, depending on the available interface.
- Binding `Element<U>`: `U` is called a manipulator. `U` must define `void operator () (T&)` (the return type doesn't actually matter). A `new U()` will be created then edited by subsequent inner XML. When the element completes, it will be "called" with the parent instance as argument. The instance of `U` will be deleted after the process is complete.
- Binding `Element<T>`: this is a "transient binding". The currently edited value is propagated.


