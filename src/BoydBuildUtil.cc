#include <cstddef>
#include <cstring>
#include <fstream>

int main(int argc, char **argv)
{
    std::ofstream outfile(argv[1]);
    if(!outfile)
    {
        fprintf(stderr, "Failed to open %s for writing!\n", argv[1]);
        return 1;
    }

    // HACK to get sensible filepaths (= starting from this folder) from __FILE__ for logging.
    // BoydBuildUtil is compiled from src/BoydBuildUtil.cc; so how many chars are from the start of __FILE__ to
    // "BoydBuildUtil.cc" in the string is how many chars are to be skipped when logging __FILE__!
    const char *nameLoc = strstr(__FILE__, "BoydBuildUtil.cc");
    size_t nameLocOffset = nameLoc ? (nameLoc - __FILE__) : 0;
    outfile << "#define BoydEngine__FILE__OFFSET " << nameLocOffset;

    return 0;
}
