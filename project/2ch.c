extern size_t check_local_account(void)
{
    size_t res = 0;
    const char *homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug("homedir is %s\n", homedir);

    const char* dvach_subdir = "/.mod2ch";
    const char* dvach_account_file = "/.creds";

    size_t fullpathsize = strlen(homedir) + strlen(dvach_subdir) + strlen(dvach_account_file) + 2;
    char *fullpath = malloc(fullpathsize);
    assert(fullpath);

    memset(fullpath, 0, fullpathsize);

    strncpy(fullpath, homedir, strlen(homedir));

    struct stat stat_buf = {0};

    strncat(fullpath, dvach_subdir, strlen(dvach_subdir));
    debug("local folder with credentials is %s\n", fullpath);

    if ((res = stat(fullpath, &stat_buf))) {
        debug("cannot access local folder with credentials %s (%d)\n", fullpath, errno);
        return res;
    }

    if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
        res = -1;
        debug("%s is not a directory\n", fullpath);
        return res;
    }

    strncat(fullpath, dvach_account_file, strlen(dvach_account_file));
    debug("local file with credentials is %s\n", fullpath);

    if ((res = stat(fullpath, &stat_buf))) {
        debug("cannot access local file with credentials %s (%d)\n", fullpath, errno);
        return res;
    }

    if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
        res = -1;
        debug("%s is not a file\n", fullpath);
        return res;
    }

    return res;
}


extern size_t check_local_file(const char* path)
{
    const char* check_path = path;
    assert(check_path);
    debug("checking path %s\n", check_path);

    size_t stat_res = 0;
    struct stat stat_buf = {0};

    if ((stat_res = stat(check_path, &stat_buf))) {
        debug("%d\n", errno);
    }

    return stat_res;
}


//extern size_t session_init(void)
//{
//
//
//    assert(check_path);
//    debug("checking path %s\n", check_path);
//
//    size_t stat_res = 0;
//    struct stat stat_buf = {0};
//
//    if ((stat_res = stat(check_path, &stat_buf))) {
//        debug("%d\n", errno);
//    }
//
//    return stat_res;
//}
