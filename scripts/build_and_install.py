#!/usr/bin/env python3

import os
import multiprocessing
import shutil
import subprocess
import sys
import getopt




def usage():
    print('options for building completable:\n')
    print('    -h, --help                print this message\n')
    print('    -b  --build_dir           build directory')
    print('                                * defaults to <completable root>/build')
    print('                                * relative paths are relative to <completable root>\n')
    print('    -i  --install_dir         install directory')
    print('                                * defaults to <completable root>/install')
    print('                                * relative paths are relative to build_dir\n')
    print('    -m  --matchmaker_dir      location of installed matchmaker cmake directory')
    print('                                * defaults to <completable root>/matchmaker/install/'           \
                                                         'matchmaker/lib/cmake')
    print('                                * relative paths are relative to <completable root>')
    print('                                * ignored if dynamic_loading (\'-l\')\n')
    print('    -q  --q                   use if linking against a matchmaker library built with q only to:')
    print('                                * include matchmaker_q/matchmaker.h instead of ' +              \
            'matchmaker/matchmaker.h')
    print('                                * change default matchmaker_dir to: ' +                         \
                                             '<completable_root>/matchmaker/install/matchmaker_q/lib/cmake')
    print('                                * ignored if dynamic_loading (\'-l\')\n')
    print('    -c  --clang               force use of clang compiler (defaults to system compiler)\n')
    print('    -d  --debug               debug build (default is release)\n')
    print('    -l  --dynamic_loading     load matchmaker at runtime')
    print('                                * avoids linking to matchmaker\n')


def build_and_install(build_dir, install_dir, matchmaker_dir, use_clang, q, debug, dynamic_loading):
    start_dir = os.getcwd()

    completable_root = os.path.dirname(os.path.realpath(__file__)) + '/../'
    os.chdir(completable_root)

    if build_dir == '':
        build_dir = completable_root + '/build'
    while build_dir[-1] == '/':
        build_dir = build_dir[:-1]
    if q:
        build_dir = build_dir + '_q'

    if install_dir == '':
        install_dir = completable_root + '/install'
    while install_dir[-1] == '/':
        install_dir = install_dir[:-1]

    if matchmaker_dir == '':
        matchmaker_dir = completable_root + '/matchmaker/install/lib/matchmaker'
        if q:
            matchmaker_dir = matchmaker_dir + '_q'
        matchmaker_dir = matchmaker_dir + '/cmake'

    shutil.rmtree(build_dir, ignore_errors=True)
    os.makedirs(build_dir)
    os.chdir(build_dir)

    if not os.path.exists(install_dir):
        os.makedirs(install_dir)

    cmake_cmd = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + install_dir]
    if use_clang:
        cmake_cmd.append('-DCMAKE_C_COMPILER=/usr/bin/clang')
        cmake_cmd.append('-DCMAKE_CXX_COMPILER=/usr/bin/clang++')

    if debug:
        cmake_cmd.append('-DCMAKE_BUILD_TYPE=Debug')
    else:
        cmake_cmd.append('-DCMAKE_BUILD_TYPE=Release')

    if dynamic_loading:
        cmake_cmd.append('-Dmatchmaker_DL=ON')
    else:
        cmake_cmd.append('-Dmatchmaker_DL=OFF')
        cmake_cmd.append('-Dmatchmaker_DIR=' + matchmaker_dir)

    cmake_cmd.append(completable_root)

    if subprocess.run(cmake_cmd).returncode != 0:
        print('cmake failed')
        os.chdir(start_dir)
        exit(1)

    if subprocess.run(['make', '-j' + str(multiprocessing.cpu_count()), 'install']).returncode != 0:
        print('make failed')
        os.chdir(start_dir)
        exit(1)

    os.chdir(start_dir)



def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hcb:i:m:qdl',
                ['help', 'clang', 'build_dir', 'install_dir', 'matchmaker_dir', 'q', 'debug',
                 'dynamic_loading'])
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(2)

    use_clang = False
    build_dir = ''
    install_dir = ''
    matchmaker_dir = ''
    q = False
    debug = False
    dynamic_loading = False

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ('-c', '--clang'):
            use_clang = True
        elif o in ('-b', '--build_dir'):
            build_dir = a
        elif o in ('-i', '--install_dir'):
            install_dir = a
        elif o in ('-m', '--matchmaker_dir'):
            matchmaker_dir = a
        elif o in ('-q', '--q'):
            q = True
        elif o in ('-d', '--debug'):
            debug = True
        elif o in ('-l', '--dynamic_loading'):
            dynamic_loading = True
        else:
            assert False, "unhandled option"


    build_and_install(build_dir, install_dir, matchmaker_dir, use_clang, q, debug, dynamic_loading)

    exit(0)



if __name__ == "__main__":
    main()
