//
// Created by sams on 3/4/26.
//

#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

void mkdirs_for_file(const char *filepath);
int file_exists(const char *path);

/*
 * Download file from URL if it doesn't exist at path.
 * Path is relative to $HOME (e.g., ".local/share/SubOptimal/model.bin").
 *
 * rel_path  - path relative to $HOME
 * url       - URL to download from if file missing
 *
 * Returns 0 on success (file exists or downloaded), -1 on failure.
 */
int curl_if_not_present(const char *rel_path, const char *url);

#endif //FILE_HELPERS_H
