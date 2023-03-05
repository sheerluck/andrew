from yt_dlp     import YoutubeDL as with_1786_extractors
from yt_dlp_nao import YoutubeDL as with_1821_extractors

with with_1786_extractors() as y1786, \
     with_1821_extractors() as y1821:
    set1 = y1786.add_default_info_extractors()
    set2 = y1821.add_default_info_extractors()
    dx = len(set2) - len(set1)
    print(f"Here is {dx} additional extractors:")
    for i, ex in enumerate(sorted(set2 - set1), start=1):
        print(f"{i:>3}) {ex}")
