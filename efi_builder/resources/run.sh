#!/bin/sh

mpv --vo=drm --loop-file=inf --ao=alsa --audio-device="alsa/default:CARD=Generic" /rickrollmini.mkv &
wait