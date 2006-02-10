mkdir gtkradiant-1.5.0
cp GtkRadiant/* gtkradiant-1.5.0
mkdir gtkradiant-1.5.0/radiant
cp -R GtkRadiant/radiant/* gtkradiant-1.5.0/radiant
mkdir gtkradiant-1.5.0/plugins
cp -R GtkRadiant/plugins/* gtkradiant-1.5.0/plugins
mkdir gtkradiant-1.5.0/libs
cp -R GtkRadiant/libs/* gtkradiant-1.5.0/libs
mkdir gtkradiant-1.5.0/include
cp -R GtkRadiant/include/* gtkradiant-1.5.0/include
mkdir gtkradiant-1.5.0/contrib
cp -R GtkRadiant/contrib/* gtkradiant-1.5.0/contrib
mkdir gtkradiant-1.5.0/tools
cp -R GtkRadiant/tools/* gtkradiant-1.5.0/tools
mkdir gtkradiant-1.5.0/setup
cp -R GtkRadiant/setup/* gtkradiant-1.5.0/setup
mkdir gtkradiant-1.5.0/games
cp -R GtkRadiant/games/* gtkradiant-1.5.0/games
mkdir gtkradiant-1.5.0/docs
cp -R GtkRadiant/docs/* gtkradiant-1.5.0/docs
tar -czf gtkradiant-1.5.0.tar.gz gtkradiant-1.5.0/
rm -rf gtkradiant-1.5.0
