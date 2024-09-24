# MediaCodec
meida transcode 
如果lib/linux/x86.tar.gz下载失败，则可忽略下载x86.tar.gz ：
    错误：smudge filter lfs failed
    git lfs install --skip-smudge  #跳过smudge
    git clone  #克隆github文件，大文件为LFS的一个地址
    git lfs pull  #将大文件pull回来
    git lfs install --force    #恢复smudge

执行lib/build.sh x86 可重新生成所需的第三方库(或在release中单独下载x86.tar.gz，然后放到lib/linux/目录下)