#include <x2c/x2c.h>
#include <sstream>

DTD_START_WITH_ROOT_NAME(toto_dtd, toto, "toto-int", int)
    toto = x2c::chardata();
DTD_END(toto_dtd);

DTD_START(pouet_dtd, pouet, int)
    pouet = x2c::E(toto_dtd.root());
DTD_END(pouet_dtd);

int main(int argc, char** argv)
{
    int* x;

    std::stringstream s, s2;

    s << "<toto-int>42</toto-int>";

    std::cout << s.str() << std::endl;
    x = toto_dtd.parse(s);
    std::cout << "x = " << (*x) << std::endl;

    s2 << "<pouet><toto-int>23</toto-int></pouet>";

    std::cout << s2.str() << std::endl;
    x = pouet_dtd.parse(s2);
    std::cout << "x = " << (*x) << std::endl;

    return 0;
}

