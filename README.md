structured
==========

Statically structured expressions.


Rationale
=========

This library is motivated by the need to describe an /X/ML document structure
in order to perform automatic data binding upon reading a document.
Taking advantage of the C++11 standard, it defines the following tags to
describe a structure of any given payload :

- single (cardinality 1-1)
- multiple (cardinality 1-*)
- optional&lt;single&gt; (cardinality 0-1)
- optional&lt;multiple&gt; (cardinality 0-*)
- ordered_sequence (result of (a & b))
- unordered_sequence (result of (a, b))
- alternative (result of (a | b))

Those seven tags are used in a tuple-like structure combination&lt;tag, Elements...&gt;.
The elements of ordered_sequence, unordered_sequence, and alternative are
obviously any of the six other possible combination&lt;tag&gt;. They are transparently
combined with themselves, so (a & (b & c)) and ((a & b) & c) yield the same result,
a 3-uple ordered sequence containing a, b, and c.

The struct combination&lt;tag&gt; implements a template method transform&lt;TransformerFactory&gt;(),
where TransformerFactory must implement a static method transform(PayloadType) for each
of the occurring payload types, that results in another combination&lt;tag&gt; object containing
the same structure but with transformed payloads.

