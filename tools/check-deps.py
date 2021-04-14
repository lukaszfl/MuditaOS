#!/usr/bin/env python3

import os.path
import subprocess
import sys


components = [
    ['module-bsp', 'board', 'module-vfs', 'module-os'],
    ['module-audio', 'module-bluetooth',
        'module-cellular', 'module-db', 'module-lwip', 'module-utils', 'module-gui'],
    ['module-services', 'module-sys'],
    ['module-apps'],
    ['source']
]


def generate_allowed_conenctions():
    allowed = []
    lower_layers = []
    for layer in components:
        lower_layers += layer
        for c in layer:
            for other in lower_layers:
                if c == other:
                    continue
                allowed.append((c, other))
    return allowed


allowed_connections = generate_allowed_conenctions()


def usage(progname):
    print("Usage:", progname, "builddir")


def get_ninja_deps(builddir):
    cmd = "ninja -C " + builddir + " -t deps"
    print("Executing:", cmd)
    result = subprocess.run(cmd.split(), stdout=subprocess.PIPE)

    alldeps = {}
    newfile = True
    filename = ""
    deps = []
    for line in result.stdout.decode('utf-8').split('\n'):
        # dumb parsing
        if newfile:
            if len(line) == 0:
                continue
            filename = os.path.realpath(builddir + "/" + line.split(':')[0])
            newfile = False

        if line.startswith('    '):
            deps.append(os.path.realpath(builddir + "/" + line.strip()))
            continue

        if len(line) == 0:
            alldeps[filename] = deps
            newfile = True
            filename = ""
            deps = []

    return alldeps


def get_module_name(projectpath, builddir, filename):
    if filename.startswith(builddir):
        localname = filename[len(builddir) + 1:]
    elif filename.startswith(projectpath):
        localname = filename[len(projectpath) + 1:]
    else:
        return "global", filename

    return localname.split('/')[0], localname


def main(args):
    if len(args) != 2:
        usage(args[0])
        return 1

    builddir = os.path.realpath(args[1])

    if not os.path.isdir(builddir):
        print(builddir, "is not a directory")
        return 2

    if not os.path.isfile(builddir + "/.ninja_deps"):
        print(builddir, "is not a Ninja build directory")
        return 2

    projectdir = os.path.realpath(os.path.curdir)
    deps = get_ninja_deps(builddir)

    # process deps
    for _, objdeps in deps.items():
        src = objdeps[0]
        module, localname = get_module_name(projectdir, builddir, src)
        for dep in objdeps[1:]:
            depmodule, deplocalname = get_module_name(
                projectdir, builddir, dep)
            if module == depmodule:
                continue
            if (module, depmodule) not in allowed_connections:
                print(module + " -> " + depmodule+"\t" +
                      localname+" -> " + deplocalname)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
