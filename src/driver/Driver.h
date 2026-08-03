#ifndef DRIVER_H_
#define DRIVER_H_

class Driver {
public:
    // Returns 0 on success, non-zero if any translation unit or link step failed.
    // Non-zero status is required when trans is used as make's CC.
    int run(int argc, char **argv) const;
};

#endif // DRIVER_H_

