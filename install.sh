filename=HTYFileManager
s="[Desktop Entry]\nName=海天鹰文件管理器\nComment=海天鹰文件管理器\nExec=`pwd`/$filename %u\nIcon=`pwd`/icon.png\nPath=`pwd`\nTerminal=false\nType=Application\nMimeType=inode/directory;\nCategories=System;"
echo -e $s > $filename.desktop
cp `pwd`/$filename.desktop ~/.local/share/applications/$filename.desktop