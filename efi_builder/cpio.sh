#!/bin/bash
set -euo pipefail
cd "$(dirname "$(realpath "$0")")"

# Reset for new generation
#mkdir -p ./obj
#rm -rf ./obj/vmlinuz
#rm -rf ./obj/cpio
#mkdir -p ./obj/cpio
cd ./obj/cpio

# Create mount points
#mkdir -p -m 700 ./proc
#mkdir -p -m 700 ./sys
#mkdir -p -m 700 ./dev

# Create basic file structure
#mkdir -p -m 700 ./usr/bin
#mkdir -p -m 700 ./usr/share
#mkdir -p -m 700 ./usr/lib

# Make symlinks
#ln -s usr/bin ./bin
#ln -s usr/bin ./sbin
#ln -s bin ./usr/sbin
#ln -s usr/share ./share
#ln -s usr/lib ./lib
#ln -s usr/lib ./lib64
#ln -s lib ./usr/lib64

# Copy and extract kernel modules and kernel
mkdir -p -m 700 ./usr/lib/modules
cp -r "/usr/lib/modules/$(uname -r)/" "./usr/lib/modules/$(uname -r)/"
mv "./usr/lib/modules/$(uname -r)/vmlinuz" ../vmlinuz
depmod -a --basedir="./" --moduledir="./usr/lib/modules"
cp -r ../../resources/firmware ./usr/lib/firmware

# Copy programs but save init and mystery for later
cp "$(which busybox)" ./usr/bin/busybox
chmod 700 ./usr/bin/busybox
while IFS= read -r line; do
    busybinPath="./usr/bin/$line"
    if [ ! -f "$busybinPath" ]; then
        ln -s busybox "$busybinPath"
    fi
done < <(busybox --list)

# cp /usr/bin/kmod ./usr/bin/kmod
# ln -s -f kmod ./usr/bin/depmod
# ln -s -f kmod ./usr/bin/modprobe
# ln -s -f kmod ./usr/bin/lsmod
# ln -s -f kmod ./usr/bin/modinfo
# cp /usr/lib/libzstd.so.1 ./usr/lib/libzstd.so.1
# cp /usr/lib/liblzma.so.5 ./usr/lib/liblzma.so.5
# cp /usr/lib/libz.so.1 ./usr/lib/libz.so.1
# cp /usr/lib/libcrypto.so.3 ./usr/lib/libcrypto.so.3
# cp /usr/lib/libgcc_s.so.1 ./usr/lib/libgcc_s.so.1
# cp /usr/lib/libc.so.6 ./usr/lib/libc.so.6
# cp /usr/lib/libbrotlienc.so.1 ./usr/lib/libbrotlienc.so.1
# cp /usr/lib/libbrotlidec.so.1 ./usr/lib/libbrotlidec.so.1
# cp /usr/lib64/ld-linux-x86-64.so.2 ./usr/lib64/ld-linux-x86-64.so.2
# cp /usr/lib/libm.so.6 ./usr/lib/libm.so.6
# cp /usr/lib/libbrotlicommon.so.1 ./usr/lib/libbrotlicommon.so.1
# 
# cp /usr/bin/mpv ./usr/bin/mpv
# mkdir -p -m 700 ./usr/lib/pulseaudio
# cp /usr/lib/libmujs.so ./usr/lib/libmujs.so
# cp /usr/lib/libass.so.9 ./usr/lib/libass.so.9
# cp /usr/lib/libavcodec.so.62 ./usr/lib/libavcodec.so.62
# cp /usr/lib/libavfilter.so.11 ./usr/lib/libavfilter.so.11
# cp /usr/lib/libavformat.so.62 ./usr/lib/libavformat.so.62
# cp /usr/lib/libavutil.so.60 ./usr/lib/libavutil.so.60
# cp /usr/lib/libplacebo.so.360 ./usr/lib/libplacebo.so.360
# cp /usr/lib/libswresample.so.6 ./usr/lib/libswresample.so.6
# cp /usr/lib/libswscale.so.9 ./usr/lib/libswscale.so.9
# cp /usr/lib/libcdio_paranoia.so.2 ./usr/lib/libcdio_paranoia.so.2
# cp /usr/lib/libcdio_cdda.so.2 ./usr/lib/libcdio_cdda.so.2
# cp /usr/lib/libcdio.so.19 ./usr/lib/libcdio.so.19
# cp /usr/lib/libm.so.6 ./usr/lib/libm.so.6
# cp /usr/lib/libdvdnav.so.4 ./usr/lib/libdvdnav.so.4
# cp /usr/lib/liblcms2.so.2 ./usr/lib/liblcms2.so.2
# cp /usr/lib/libarchive.so.13 ./usr/lib/libarchive.so.13
# cp /usr/lib/libavdevice.so.62 ./usr/lib/libavdevice.so.62
# cp /usr/lib/libbluray.so.3 ./usr/lib/libbluray.so.3
# cp /usr/lib/libluajit-5.1.so.2 ./usr/lib/libluajit-5.1.so.2
# cp /usr/lib/librubberband.so.3 ./usr/lib/librubberband.so.3
# cp /usr/lib/libSDL2-2.0.so.0 ./usr/lib/libSDL2-2.0.so.0
# cp /usr/lib/libuchardet.so.0 ./usr/lib/libuchardet.so.0
# cp /usr/lib/libvapoursynth-script.so.0 ./usr/lib/libvapoursynth-script.so.0
# cp /usr/lib/libXfixes.so.3 ./usr/lib/libXfixes.so.3
# cp /usr/lib/libX11.so.6 ./usr/lib/libX11.so.6
# cp /usr/lib/libzimg.so.2 ./usr/lib/libzimg.so.2
# cp /usr/lib/libz.so.1 ./usr/lib/libz.so.1
# cp /usr/lib/libasound.so.2 ./usr/lib/libasound.so.2
# cp /usr/lib/libjack.so.0 ./usr/lib/libjack.so.0
# cp /usr/lib/libopenal.so.1 ./usr/lib/libopenal.so.1
# cp /usr/lib/libpipewire-0.3.so.0 ./usr/lib/libpipewire-0.3.so.0
# cp /usr/lib/libpulse.so.0 ./usr/lib/libpulse.so.0
# cp /usr/lib/libsndio.so.7 ./usr/lib/libsndio.so.7
# cp /usr/lib/libdrm.so.2 ./usr/lib/libdrm.so.2
# cp /usr/lib/libdisplay-info.so.3 ./usr/lib/libdisplay-info.so.3
# cp /usr/lib/libgbm.so.1 ./usr/lib/libgbm.so.1
# cp /usr/lib/libjpeg.so.8 ./usr/lib/libjpeg.so.8
# cp /usr/lib/libsixel.so.1 ./usr/lib/libsixel.so.1
# cp /usr/lib/libwayland-client.so.0 ./usr/lib/libwayland-client.so.0
# cp /usr/lib/libwayland-cursor.so.0 ./usr/lib/libwayland-cursor.so.0
# cp /usr/lib/libxkbcommon.so.0 ./usr/lib/libxkbcommon.so.0
# cp /usr/lib/libXss.so.1 ./usr/lib/libXss.so.1
# cp /usr/lib/libXext.so.6 ./usr/lib/libXext.so.6
# cp /usr/lib/libXpresent.so.1 ./usr/lib/libXpresent.so.1
# cp /usr/lib/libXrandr.so.2 ./usr/lib/libXrandr.so.2
# cp /usr/lib/libXv.so.1 ./usr/lib/libXv.so.1
# cp /usr/lib/libGL.so.1 ./usr/lib/libGL.so.1
# cp /usr/lib/libEGL.so.1 ./usr/lib/libEGL.so.1
# cp /usr/lib/libwayland-egl.so.1 ./usr/lib/libwayland-egl.so.1
# cp /usr/lib/libvulkan.so.1 ./usr/lib/libvulkan.so.1
# cp /usr/lib/libva-drm.so.2 ./usr/lib/libva-drm.so.2
# cp /usr/lib/libva.so.2 ./usr/lib/libva.so.2
# cp /usr/lib/libva-wayland.so.2 ./usr/lib/libva-wayland.so.2
# cp /usr/lib/libva-x11.so.2 ./usr/lib/libva-x11.so.2
# cp /usr/lib/libvdpau.so.1 ./usr/lib/libvdpau.so.1
# cp /usr/lib/libgcc_s.so.1 ./usr/lib/libgcc_s.so.1
# cp /usr/lib/libc.so.6 ./usr/lib/libc.so.6
# cp /usr/lib/libfreetype.so.6 ./usr/lib/libfreetype.so.6
# cp /usr/lib/libfribidi.so.0 ./usr/lib/libfribidi.so.0
# cp /usr/lib/libharfbuzz.so.0 ./usr/lib/libharfbuzz.so.0
# cp /usr/lib/libunibreak.so.7 ./usr/lib/libunibreak.so.7
# cp /usr/lib/libfontconfig.so.1 ./usr/lib/libfontconfig.so.1
# cp /usr/lib/libvpx.so.12 ./usr/lib/libvpx.so.12
# cp /usr/lib/libwebpmux.so.3 ./usr/lib/libwebpmux.so.3
# cp /usr/lib/liblzma.so.5 ./usr/lib/liblzma.so.5
# cp /usr/lib/libdav1d.so.7 ./usr/lib/libdav1d.so.7
# cp /usr/lib/libopencore-amrwb.so.0 ./usr/lib/libopencore-amrwb.so.0
# cp /usr/lib/librsvg-2.so.2 ./usr/lib/librsvg-2.so.2
# cp /usr/lib/libcairo.so.2 ./usr/lib/libcairo.so.2
# cp /usr/lib/libgobject-2.0.so.0 ./usr/lib/libgobject-2.0.so.0
# cp /usr/lib/libglib-2.0.so.0 ./usr/lib/libglib-2.0.so.0
# cp /usr/lib/libsnappy.so.1 ./usr/lib/libsnappy.so.1
# cp /usr/lib/libaom.so.3 ./usr/lib/libaom.so.3
# cp /usr/lib/libgsm.so.1 ./usr/lib/libgsm.so.1
# cp /usr/lib/libjxl.so.0.11 ./usr/lib/libjxl.so.0.11
# cp /usr/lib/libjxl_threads.so.0.11 ./usr/lib/libjxl_threads.so.0.11
# cp /usr/lib/libmp3lame.so.0 ./usr/lib/libmp3lame.so.0
# cp /usr/lib/libopencore-amrnb.so.0 ./usr/lib/libopencore-amrnb.so.0
# cp /usr/lib/libopenjp2.so.7 ./usr/lib/libopenjp2.so.7
# cp /usr/lib/libopus.so.0 ./usr/lib/libopus.so.0
# cp /usr/lib/librav1e.so.0.8 ./usr/lib/librav1e.so.0.8
# cp /usr/lib/libspeex.so.1 ./usr/lib/libspeex.so.1
# cp /usr/lib/libSvtAv1Enc.so.4 ./usr/lib/libSvtAv1Enc.so.4
# cp /usr/lib/libtheoraenc.so.2 ./usr/lib/libtheoraenc.so.2
# cp /usr/lib/libtheoradec.so.2 ./usr/lib/libtheoradec.so.2
# cp /usr/lib/libvorbis.so.0 ./usr/lib/libvorbis.so.0
# cp /usr/lib/libvorbisenc.so.2 ./usr/lib/libvorbisenc.so.2
# cp /usr/lib/libwebp.so.7 ./usr/lib/libwebp.so.7
# cp /usr/lib/libx264.so.165 ./usr/lib/libx264.so.165
# cp /usr/lib/libx265.so.215 ./usr/lib/libx265.so.215
# cp /usr/lib/libxvidcore.so.4 ./usr/lib/libxvidcore.so.4
# cp /usr/lib/libvpl.so.2 ./usr/lib/libvpl.so.2
# cp /usr/lib/libbs2b.so.0 ./usr/lib/libbs2b.so.0
# cp /usr/lib/libvmaf.so.3 ./usr/lib/libvmaf.so.3
# cp /usr/lib/libvidstab.so.1.2 ./usr/lib/libvidstab.so.1.2
# cp /usr/lib/libzmq.so.5 ./usr/lib/libzmq.so.5
# cp /usr/lib/libglslang.so.16 ./usr/lib/libglslang.so.16
# cp /usr/lib/libOpenCL.so.1 ./usr/lib/libOpenCL.so.1
# cp /usr/lib/libxml2.so.16 ./usr/lib/libxml2.so.16
# cp /usr/lib/libbz2.so.1.0 ./usr/lib/libbz2.so.1.0
# cp /usr/lib/libdvdread.so.8 ./usr/lib/libdvdread.so.8
# cp /usr/lib/libmodplug.so.1 ./usr/lib/libmodplug.so.1
# cp /usr/lib/libopenmpt.so.0 ./usr/lib/libopenmpt.so.0
# cp /usr/lib/libgmp.so.10 ./usr/lib/libgmp.so.10
# cp /usr/lib/libgnutls.so.30 ./usr/lib/libgnutls.so.30
# cp /usr/lib/libsrt.so.1.5 ./usr/lib/libsrt.so.1.5
# cp /usr/lib/libssh.so.4 ./usr/lib/libssh.so.4
# cp /usr/lib/libunwind.so.8 ./usr/lib/libunwind.so.8
# cp /usr/lib/libshaderc_shared.so.1 ./usr/lib/libshaderc_shared.so.1
# cp /usr/lib/libglslang-default-resource-limits.so.16 ./usr/lib/libglslang-default-resource-limits.so.16
# cp /usr/lib/libdovi.so.3 ./usr/lib/libdovi.so.3
# cp /usr/lib/libstdc++.so.6 ./usr/lib/libstdc++.so.6
# cp /usr/lib/libsoxr.so.0 ./usr/lib/libsoxr.so.0
# cp /usr/lib64/ld-linux-x86-64.so.2 ./usr/lib64/ld-linux-x86-64.so.2
# cp /usr/lib/libcrypto.so.3 ./usr/lib/libcrypto.so.3
# cp /usr/lib/libacl.so.1 ./usr/lib/libacl.so.1
# cp /usr/lib/libzstd.so.1 ./usr/lib/libzstd.so.1
# cp /usr/lib/liblz4.so.1 ./usr/lib/liblz4.so.1
# cp /usr/lib/libraw1394.so.11 ./usr/lib/libraw1394.so.11
# cp /usr/lib/libavc1394.so.0 ./usr/lib/libavc1394.so.0
# cp /usr/lib/librom1394.so.0 ./usr/lib/librom1394.so.0
# cp /usr/lib/libiec61883.so.0 ./usr/lib/libiec61883.so.0
# cp /usr/lib/libxcb.so.1 ./usr/lib/libxcb.so.1
# cp /usr/lib/libxcb-shm.so.0 ./usr/lib/libxcb-shm.so.0
# cp /usr/lib/libxcb-shape.so.0 ./usr/lib/libxcb-shape.so.0
# cp /usr/lib/libxcb-xfixes.so.0 ./usr/lib/libxcb-xfixes.so.0
# cp /usr/lib/libv4l2.so.0 ./usr/lib/libv4l2.so.0
# cp /usr/lib/libfftw3.so.3 ./usr/lib/libfftw3.so.3
# cp /usr/lib/libsamplerate.so.0 ./usr/lib/libsamplerate.so.0
# cp /usr/lib/pulseaudio/libpulsecommon-17.0.so ./usr/lib/pulseaudio/libpulsecommon-17.0.so
# cp /usr/lib/libdbus-1.so.3 ./usr/lib/libdbus-1.so.3
# cp /usr/lib/libbsd.so.0 ./usr/lib/libbsd.so.0
# cp /usr/lib/libexpat.so.1 ./usr/lib/libexpat.so.1
# cp /usr/lib/libcurl.so.4 ./usr/lib/libcurl.so.4
# cp /usr/lib/libpng16.so.16 ./usr/lib/libpng16.so.16
# cp /usr/lib/libgdk_pixbuf-2.0.so.0 ./usr/lib/libgdk_pixbuf-2.0.so.0
# cp /usr/lib/libffi.so.8 ./usr/lib/libffi.so.8
# cp /usr/lib/libXrender.so.1 ./usr/lib/libXrender.so.1
# cp /usr/lib/libGLdispatch.so.0 ./usr/lib/libGLdispatch.so.0
# cp /usr/lib/libGLX.so.0 ./usr/lib/libGLX.so.0
# cp /usr/lib/libX11-xcb.so.1 ./usr/lib/libX11-xcb.so.1
# cp /usr/lib/libxcb-dri3.so.0 ./usr/lib/libxcb-dri3.so.0
# cp /usr/lib/libbrotlidec.so.1 ./usr/lib/libbrotlidec.so.1
# cp /usr/lib/libgraphite2.so.3 ./usr/lib/libgraphite2.so.3
# cp /usr/lib/libgio-2.0.so.0 ./usr/lib/libgio-2.0.so.0
# cp /usr/lib/libpangocairo-1.0.so.0 ./usr/lib/libpangocairo-1.0.so.0
# cp /usr/lib/libpango-1.0.so.0 ./usr/lib/libpango-1.0.so.0
# cp /usr/lib/libxcb-render.so.0 ./usr/lib/libxcb-render.so.0
# cp /usr/lib/libpixman-1.so.0 ./usr/lib/libpixman-1.so.0
# cp /usr/lib/libpcre2-8.so.0 ./usr/lib/libpcre2-8.so.0
# cp /usr/lib/libjxl_cms.so.0.11 ./usr/lib/libjxl_cms.so.0.11
# cp /usr/lib/libhwy.so.1 ./usr/lib/libhwy.so.1
# cp /usr/lib/libbrotlienc.so.1 ./usr/lib/libbrotlienc.so.1
# cp /usr/lib/libmpg123.so.0 ./usr/lib/libmpg123.so.0
# cp /usr/lib/libogg.so.0 ./usr/lib/libogg.so.0
# cp /usr/lib/libsharpyuv.so.0 ./usr/lib/libsharpyuv.so.0
# cp /usr/lib/libmvec.so.1 ./usr/lib/libmvec.so.1
# cp /usr/lib/libgomp.so.1 ./usr/lib/libgomp.so.1
# cp /usr/lib/libsodium.so.26 ./usr/lib/libsodium.so.26
# cp /usr/lib/libpgm-5.3.so.0 ./usr/lib/libpgm-5.3.so.0
# cp /usr/lib/libSPIRV-Tools-opt.so ./usr/lib/libSPIRV-Tools-opt.so
# cp /usr/lib/libSPIRV-Tools.so ./usr/lib/libSPIRV-Tools.so
# cp /usr/lib/libicuuc.so.78 ./usr/lib/libicuuc.so.78
# cp /usr/lib/libvorbisfile.so.3 ./usr/lib/libvorbisfile.so.3
# cp /usr/lib/libleancrypto.so.1 ./usr/lib/libleancrypto.so.1
# cp /usr/lib/libp11-kit.so.0 ./usr/lib/libp11-kit.so.0
# cp /usr/lib/libidn2.so.0 ./usr/lib/libidn2.so.0
# cp /usr/lib/libunistring.so.5 ./usr/lib/libunistring.so.5
# cp /usr/lib/libtasn1.so.6 ./usr/lib/libtasn1.so.6
# cp /usr/lib/libhogweed.so.7 ./usr/lib/libhogweed.so.7
# cp /usr/lib/libnettle.so.9 ./usr/lib/libnettle.so.9
# cp /usr/lib/libXau.so.6 ./usr/lib/libXau.so.6
# cp /usr/lib/libXdmcp.so.6 ./usr/lib/libXdmcp.so.6
# cp /usr/lib/libv4lconvert.so.0 ./usr/lib/libv4lconvert.so.0
# cp /usr/lib/libsndfile.so.1 ./usr/lib/libsndfile.so.1
# cp /usr/lib/libsystemd.so.0 ./usr/lib/libsystemd.so.0
# cp /usr/lib/libasyncns.so.0 ./usr/lib/libasyncns.so.0
# cp /usr/lib/libmd.so.0 ./usr/lib/libmd.so.0
# cp /usr/lib/libnghttp3.so.9 ./usr/lib/libnghttp3.so.9
# cp /usr/lib/libngtcp2_crypto_ossl.so.0 ./usr/lib/libngtcp2_crypto_ossl.so.0
# cp /usr/lib/libngtcp2.so.16 ./usr/lib/libngtcp2.so.16
# cp /usr/lib/libnghttp2.so.14 ./usr/lib/libnghttp2.so.14
# cp /usr/lib/libssh2.so.1 ./usr/lib/libssh2.so.1
# cp /usr/lib/libpsl.so.5 ./usr/lib/libpsl.so.5
# cp /usr/lib/libssl.so.3 ./usr/lib/libssl.so.3
# cp /usr/lib/libgssapi_krb5.so.2 ./usr/lib/libgssapi_krb5.so.2
# cp /usr/lib/libgmodule-2.0.so.0 ./usr/lib/libgmodule-2.0.so.0
# cp /usr/lib/libglycin-2.so.0 ./usr/lib/libglycin-2.so.0
# cp /usr/lib/libbrotlicommon.so.1 ./usr/lib/libbrotlicommon.so.1
# cp /usr/lib/libmount.so.1 ./usr/lib/libmount.so.1
# cp /usr/lib/libpangoft2-1.0.so.0 ./usr/lib/libpangoft2-1.0.so.0
# cp /usr/lib/libthai.so.0 ./usr/lib/libthai.so.0
# cp /usr/lib/libicudata.so.78 ./usr/lib/libicudata.so.78
# cp /usr/lib/libFLAC.so.14 ./usr/lib/libFLAC.so.14
# cp /usr/lib/libkrb5.so.3 ./usr/lib/libkrb5.so.3
# cp /usr/lib/libk5crypto.so.3 ./usr/lib/libk5crypto.so.3
# cp /usr/lib/libcom_err.so.2 ./usr/lib/libcom_err.so.2
# cp /usr/lib/libkrb5support.so.0 ./usr/lib/libkrb5support.so.0
# cp /usr/lib/libkeyutils.so.1 ./usr/lib/libkeyutils.so.1
# cp /usr/lib/libresolv.so.2 ./usr/lib/libresolv.so.2
# cp /usr/lib/libseccomp.so.2 ./usr/lib/libseccomp.so.2
# cp /usr/lib/libblkid.so.1 ./usr/lib/libblkid.so.1
# cp /usr/lib/libdatrie.so.1 ./usr/lib/libdatrie.so.1