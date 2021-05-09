# youtube-dl-cleanup
Remove duplicated videos after download multiple times a playlist

# Description

When you want to update a playlist with youtube-dl, you simply run youtube-dl one more time.

But after some years, youtube-dl will randomly decide to use another output format.
So you will have some files like:

```
A Tutorial Introduction to C++11 & 14 Part 1-TK_SfTfxaxc.mkv
A Tutorial Introduction to C++11 & 14 Part 1-TK_SfTfxaxc.mp4
A Tutorial Introduction to C++11 & 14 Part 1-TK_SfTfxaxc.webm
```

But how to automatically remove old videos.

On Windows, the modification date is the date of the video on the Youtube server.
So based on the speed of the encoding, the new video may be the oldest.

The idea is to use the creation date time.
This information does not exist on Linux so this program is Windows only.

# How to build

You just need Visual Studio Community and open the project with CMake installed.

# How to use it

``youtubedl-cleanup.exe --dry-run -r path [more_path]``

  * ``--dry-run``: do not erase, just show file that will be removed.
  * ``-r``: recursively check. Usefull if you have multiple playlists in a same folder.
