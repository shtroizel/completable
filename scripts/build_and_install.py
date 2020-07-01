#!/usr/bin/env python3

import os
import multiprocessing
import shutil
import subprocess
import sys
import getopt




def usage():
    print(sys.argv[0] + ' [OPTION]')
    print('    -h, --help                print this message')
    print('    -b  --build_dir           build directory (defaults to <repo_root>/build)')
    print('    -i  --install_dir         install directory (defaults to <repo_root>/install)')
    print('                              relative paths are relative to build_dir')
    print('    -m  --matchmaker_dir      matchmaker install directory (defaults to <repo_root>/../install)')
    print('                              relative paths are relative to build_dir')
    print('    -c  --clang               force use of clang compiler (defaults to system compiler)')


def build_and_install(repo_root, build_dir, install_dir, matchmaker_dir, use_clang):
    start_dir = os.getcwd()

    if repo_root == '':
        repo_root = '../../'

    if build_dir == '':
        build_dir = repo_root + '/build/'

    if install_dir == '':
        install_dir = repo_root + '/install/'

    if matchmaker_dir == '':
        matchmaker_dir = repo_root + '/../install/'

    shutil.rmtree(build_dir, ignore_errors=True)
    os.makedirs(build_dir)
    shutil.rmtree(install_dir, ignore_errors=True)
    os.makedirs(install_dir)

    os.chdir(build_dir)

    cmake_cmd = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + install_dir]
    cmake_cmd.append('-Dmatchmaker_DIR=' + matchmaker_dir + '/lib/matchmaker/cmake')
    if use_clang:
        cmake_cmd.append('-DCMAKE_C_COMPILER=/usr/bin/clang')
        cmake_cmd.append('-DCMAKE_CXX_COMPILER=/usr/bin/clang++')

    cmake_cmd.append(repo_root)

    if subprocess.run(cmake_cmd).returncode != 0:
        print('cmake failed')
        os.chdir(repo_root)
        exit(1)

    if subprocess.run(['make', '-j' + str(multiprocessing.cpu_count()), 'install']).returncode != 0:
        print('make failed')
        os.chdir(repo_root)
        exit(1)

    os.chdir(start_dir)



def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hcb:i:m:', ['help', 'clang', 'build_dir',
                                                              'install_dir', 'matchmaker_dir'])
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(2)

    use_clang = False
    build_dir = ''
    install_dir = ''
    matchmaker_dir = ''

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
        else:
            assert False, "unhandled option"

    repo_root = os.path.dirname(os.path.realpath(__file__)) + '/../'

    build_and_install(repo_root, build_dir, install_dir, matchmaker_dir, use_clang)

    exit(0)



if __name__ == "__main__":
    main()
