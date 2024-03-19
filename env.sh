#!bin/shell
export GKFS_INSTALL_PATH=/home/changqin/gekkofs_deps/install                                                                      
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GKFS_INSTALL_PATH}/lib:${GKFS_INSTALL_PATH}/lib64
export LIBGKFS_HOSTS_FILE=/home/changqin/gekkofs_deps/install/bin/gkfs_hosts.txt

gkfs_daemon -r /home/changqin/gekkofs/fs_data -m /home/changqin/gekkofs/pseudo_gkfs_mount_dir -l ens33 -H /home/changqin/gekkofs_deps/install/bin/gkfs_hosts.txt &