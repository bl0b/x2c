#include "structure.h"

template <typename TransformerFactory>
struct transform {
    template <typename Element>
        struct dest_type {
            typedef template TransformerFactory<Element>::type;
        };

    template <class kls, typename Elements...>
        struct dest_type<combination<kls, Elements...>> {
            typedef combination<
                            kls,
                            dest_type<Elements>::type...
                        > type;
        };


};
