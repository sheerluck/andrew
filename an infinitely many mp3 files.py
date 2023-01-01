import os
import contextlib
import pendulum
import yt_dlp


def parse(ydl):
    dts = pendulum.now().to_datetime_string()
    if ydl is None:
        name = f"{dts}.mp3"
    else:
        blines = ydl.saved_stderr.split(b"\n")
        longline = [x for x in blines
                    if b"Metadata update for StreamTitle: " in x][-1]
        blines = longline.split(b"\r")
        name = blines[-1].decode("utf-8")[58:]
        name = f"{dts} {name}.mp3" 
    return yt_dlp.utils.sanitize_filename(name)


def download(URLS, ydl_opts):
    start = pendulum.now()
    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        error_code = ydl.download(URLS)
        fn = parse(ydl)
    stop = pendulum.now()
    if (stop - start).in_seconds() < 10:
        with contextlib.suppress(OSError): os.remove(fn)
        return False, None
    return True, fn


def main() -> int:
    URLS = ["https://icecast-studio21.cdnvideo.ru/KalynaK_1a"]
    fn = parse(None)
    while True:
        ydl_opts = {"verbose": True, "outtmpl": fn,
            "external_downloader": {"default": "ffmpeg"},
            "external_downloader_args": {"save_stderr": True}}
        with contextlib.suppress(OSError): os.remove(fn)
        ok, newfn = download(URLS, ydl_opts)
        if ok:
            fn = newfn
    return 0


if __name__ == "__main__":
    exit(main())
