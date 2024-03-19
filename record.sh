export GKFS_INSTALL_PATH=/home/changqin/gekkofs_deps/install                                                                      
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GKFS_INSTALL_PATH}/lib:${GKFS_INSTALL_PATH}/lib64

export LIBGKFS_HOSTS_FILE=/home/changqin/gekkofs_deps/install/bin/gkfs_hosts.txt


/home/changqin/gekkofs/build/src/daemon/gkfs_daemon -r /home/changqin/gekkofs/fs_data -m /home/changqin/gekkofs/pseudo_gkfs_mount_dir -l ens33 -H /home/changqin/gekkofs_deps/install/bin/gkfs_hosts.txt &
LD_PRELOAD=/usr/local/lib/libgkfs_intercept.so cp ~/googletest-main.zip /home/changqin/gekkofs/pseudo_gkfs_mount_dir/googletest-main.zip
LD_PRELOAD=/usr/local/lib/libgkfs_intercept.so ls /home/changqin/gekkofs/pseudo_gkfs_mount_dir/
LD_PRELOAD=/usr/local/lib/libgkfs_intercept.so md5sum ~/googletest-main.zip /home/changqin/gekkofs/pseudo_gkfs_mount_dir/googletest-main.zip


gkfs_daemon -r /home/changqin/gekkofs/fs_data -m /home/changqin/gekkofs/pseudo_gkfs_mount_dir -l ens33 -H /home/changqin/gekkofs_deps/install/bin/gkfs_hosts.txt &
LD_PRELOAD=/home/changqin/gekkofs/build/src/client/libgkfs_intercept.so ls /home/changqin/gekkofs/pseudo_gkfs_mount_dir/


"/home/changqin/gekkofs/include",
                "/home/changqin/gekkofs/external/hermes/include",
                "/home/changqin/gekkofs/external/CLI11/include",
                "/home/changqin/gekkofs/external/fmt/include",
                "/home/changqin/gekkofs/external/spdlog/include",
                "/home/changqin/gekkofs_deps/install/include"
