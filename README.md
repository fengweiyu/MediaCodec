# MediaCodec



1.linux平台



1.1.编译


1.1.1.依赖库

如果lib/linux/x86.tar.gz下载失败，则可忽略下载x86.tar.gz ：

    错误：smudge filter lfs failed
    
    git lfs install --skip-smudge  #跳过smudge
    
    git clone  #克隆github文件，大文件为LFS的一个地址
    
    git lfs pull  #将大文件pull回来
    
    git lfs install --force    #恢复smudge
    
执行lib/build.sh x86 可重新生成所需的第三方库(或双击/lib/linux/release.url单独下载x86.tar.gz，然后放到lib/linux/目录下)


1.1.2.生成MediaTranscode

执行sh build.sh x86即可在build目录下对应平台生成可执行程序MediaTranscode



1.2.使用

./MediaTranscode会使用默认的配置文件demo.json，或可在程序目录下执行其他配置文件，如 ./MediaTranscode demo_4to5g2a.json
编译程序可直接在build\linux\x86目录下双击release.url下载




2.win平台


2.1.编译

2.1.1.依赖库

双击/lib/win/release.url下载x64msvc.zip，然后解压放到lib/win/目录下)


2.1.2.生成MediaCodec.exe

打开prj\win\MediaCodec.sln工程直接生成即可


2.2.使用

MediaCodec.exe会使用默认的配置文件demo.json，或可在程序目录下执行其他配置文件，如 MediaCodec.exe demo_4to5g2a.json
编译程序可直接在build\win\x64目录下双击release.url下载



