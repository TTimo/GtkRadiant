%define name gtkradiant
%define version 1.5.0
%define release 5
%define installdir /opt/%{name}

Summary: GtkRadiant Level Editor
Name: %{name}
Version: %{version}
Release: %{release}
Group: Applications/Editors
Copyright: Id Software
URL: www.qeradiant.com
Distribution: GtkRadiant
Packager: %{packager}
Prefix: %{installdir}
Source: %{name}-%{version}.tar.gz
Requires: gtk2 >= 2.4.0 , gtkglext >= 1.0.0 , libxml2 >= 2.0.0 , zlib >= 1.2.0 , libpng >= 1.2.0 , mhash >= 0.9.0
BuildRequires: python >= 2.3.0 , gcc >= 3.0.0 , scons >= 0.96 , gtk2-devel >= 2.4.0 , gtkglext-devel >= 1.0.0 , libxml2-devel >= 2.0.0 , zlib-devel >= 1.2.0 , libpng-devel >= 1.2.0 , mhash-devel >= 0.9.0

%description
GtkRadiant is a cross-platform level editor for games based on Id Software
technology.

%prep
%setup -q

%build
scons BUILD="release" SETUP="0"

%install
echo $RPM_BUILD_ROOT/
rm -rf $RPM_BUILD_ROOT%{installdir}
mkdir -p $RPM_BUILD_ROOT%{installdir}

cp install/radiant.x86 $RPM_BUILD_ROOT%{installdir}
cp install/q3map2.x86 $RPM_BUILD_ROOT%{installdir}
cp install/q2map $RPM_BUILD_ROOT%{installdir}
cp setup/linux/bspc $RPM_BUILD_ROOT%{installdir}
cp setup/links.htm $RPM_BUILD_ROOT%{installdir}
cp setup/credits.html $RPM_BUILD_ROOT%{installdir}
cp setup/changelog.txt $RPM_BUILD_ROOT%{installdir}
cp setup/license.txt $RPM_BUILD_ROOT%{installdir}
cp setup/data/tools/global.xlink $RPM_BUILD_ROOT%{installdir}
mkdir $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/lighting_DBS_omni_fp.glsl $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/lighting_DBS_omni_vp.glsl $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/zfill_fp.glsl $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/zfill_vp.glsl $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/lighting_DBS_omni_fp.glp $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/lighting_DBS_omni_vp.glp $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/zfill_fp.glp $RPM_BUILD_ROOT%{installdir}/gl
cp setup/data/tools/gl/zfill_vp.glp $RPM_BUILD_ROOT%{installdir}/gl
cp include/RADIANT_MINOR $RPM_BUILD_ROOT%{installdir}
cp include/RADIANT_MAJOR $RPM_BUILD_ROOT%{installdir}
mkdir $RPM_BUILD_ROOT%{installdir}/et.game
mkdir $RPM_BUILD_ROOT%{installdir}/et.game/etmain
cp games/ETPack/et.game/etmain/default_shaderlist.txt $RPM_BUILD_ROOT%{installdir}/et.game/etmain
cp games/ETPack/et.game/etmain/et_entities.def $RPM_BUILD_ROOT%{installdir}/et.game/etmain
cp games/ETPack/et.game/etmain/et_entities.ent $RPM_BUILD_ROOT%{installdir}/et.game/etmain
cp games/ETPack/et.game/game.xlink $RPM_BUILD_ROOT%{installdir}/et.game
cp games/ETPack/et.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/et.game
mkdir $RPM_BUILD_ROOT%{installdir}/et.game/bitmaps
cp games/ETPack/et.game/bitmaps/splash.bmp $RPM_BUILD_ROOT%{installdir}/et.game/bitmaps
cp games/ETPack/et.game/default_build_menu.txt $RPM_BUILD_ROOT%{installdir}/et.game/bitmaps
mkdir $RPM_BUILD_ROOT%{installdir}/games
cp games/ETPack/games/et.game $RPM_BUILD_ROOT%{installdir}/games
cp games/Q1Pack/games/q1.game $RPM_BUILD_ROOT%{installdir}/games
cp games/Q2Pack/games/q2.game $RPM_BUILD_ROOT%{installdir}/games
cp games/Q3Pack/tools/games/q3.game $RPM_BUILD_ROOT%{installdir}/games
cp games/Doom3Pack/tools/games/doom3.game $RPM_BUILD_ROOT%{installdir}/games
cp games/Q4Pack/tools/games/q4.game $RPM_BUILD_ROOT%{installdir}/games
cp games/JAPack/Tools/games/ja.game $RPM_BUILD_ROOT%{installdir}/games
mkdir $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/entity.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/model.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/shaders.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/vfspk3.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/archivepak.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/mapxml.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/md3model.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/image.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/imagehl.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/archivezip.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/imagepng.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/imageq2.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/mapq3.so $RPM_BUILD_ROOT%{installdir}/modules
cp install/modules/archivewad.so $RPM_BUILD_ROOT%{installdir}/modules
mkdir $RPM_BUILD_ROOT%{installdir}/q1.game
cp games/Q1Pack/q1.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/q1.game
mkdir $RPM_BUILD_ROOT%{installdir}/q1.game/id1
cp games/Q1Pack/q1.game/id1/entities.ent $RPM_BUILD_ROOT%{installdir}/q1.game/id1
mkdir $RPM_BUILD_ROOT%{installdir}/q2.game
cp games/Q2Pack/q2.game/game.xlink $RPM_BUILD_ROOT%{installdir}/q2.game
cp games/Q2Pack/q2.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/q2.game
mkdir $RPM_BUILD_ROOT%{installdir}/q2.game/baseq2
cp games/Q2Pack/q2.game/baseq2/entities.def $RPM_BUILD_ROOT%{installdir}/q2.game/baseq2
mkdir $RPM_BUILD_ROOT%{installdir}/q3.game
cp games/Q3Pack/tools/q3.game/game.xlink $RPM_BUILD_ROOT%{installdir}/q3.game
cp games/Q3Pack/tools/q3.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/q3.game
mkdir $RPM_BUILD_ROOT%{installdir}/q3.game/missionpack
cp games/Q3Pack/tools/q3.game/missionpack/default_shaderlist.txt $RPM_BUILD_ROOT%{installdir}/q3.game/missionpack
cp games/Q3Pack/tools/q3.game/missionpack/entities-ta.def $RPM_BUILD_ROOT%{installdir}/q3.game/missionpack
cp games/Q3Pack/tools/q3.game/missionpack/entities-ta.ent $RPM_BUILD_ROOT%{installdir}/q3.game/missionpack
mkdir $RPM_BUILD_ROOT%{installdir}/q3.game/baseq3
cp games/Q3Pack/tools/q3.game/baseq3/entities.ent $RPM_BUILD_ROOT%{installdir}/q3.game/baseq3
cp games/Q3Pack/tools/q3.game/baseq3/default_shaderlist.txt $RPM_BUILD_ROOT%{installdir}/q3.game/baseq3
cp games/Q3Pack/tools/q3.game/baseq3/entities.def $RPM_BUILD_ROOT%{installdir}/q3.game/baseq3
mkdir $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/view_cubicclipping.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/cap_endcap.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/window4.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/splash.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/show_entities.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_selecttouching.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/cap_iendcap.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/patch_wireframe.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/patch_insdel.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/brush_rotatey.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/view_cameraupdate.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/dontselectcurve.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/scalelockz.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/file_open.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/ellipsis.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/brush_rotatez.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/scalelocky.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/dontselectmodel.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/modify_vertices.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/brush_rotatex.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/brush_flipy.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_selectinside.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/patch_weld.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/view_cameratoggle.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/select_mousescale.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/view_change.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_csgsubtract.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/patch_bend.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/window3.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/scalelockx.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/select_mouseresize.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/window1.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/view_clipper.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/logo.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/patch_drilldown.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/texture_lock.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/textures_popup.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_selectpartialtall.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_selectcompletetall.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/window2.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/select_mousetranslate.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/brush_flipx.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_makehollow.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/select_mouserotate.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/selection_csgmerge.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/file_save.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/popup_selection.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/cap_bevel.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/brush_flipz.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/cap_ibevel.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/curve_cap.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/modify_faces.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/notex.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/modify_edges.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/icon.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/shadernotex.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/patch_showboundingbox.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/view_entity.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/cap_cylinder.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/white.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/black.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
cp setup/data/tools/bitmaps/noFalloff.bmp $RPM_BUILD_ROOT%{installdir}/bitmaps
mkdir $RPM_BUILD_ROOT%{installdir}/doom3.game
cp games/Doom3Pack/tools/doom3.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/doom3.game
mkdir $RPM_BUILD_ROOT%{installdir}/q4.game
cp games/Q4Pack/tools/q4.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/q4.game
mkdir $RPM_BUILD_ROOT%{installdir}/ja.game
cp games/JAPack/Tools/ja.game/default_build_menu.xml $RPM_BUILD_ROOT%{installdir}/ja.game
cp games/JAPack/Tools/ja.game/game.xlink $RPM_BUILD_ROOT%{installdir}/ja.game
mkdir $RPM_BUILD_ROOT%{installdir}/ja.game/base
cp games/JAPack/Tools/ja.game/base/mp_entities.def $RPM_BUILD_ROOT%{installdir}/ja.game/base
cp games/JAPack/Tools/ja.game/base/sp_entities.def $RPM_BUILD_ROOT%{installdir}/ja.game/base
cp games/JAPack/Tools/ja.game/base/default_shaderlist.txt $RPM_BUILD_ROOT%{installdir}/ja.game/base


%clean

%files
%defattr(-,root,root)
%{installdir}
