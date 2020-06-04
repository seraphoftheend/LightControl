void listDir(fs::FS &fs, const char * dirname, uint8_t);
bool readFile(fs::FS &fs, const char *);
bool writeFile(fs::FS &fs, const char *, const char *);
void appendFile(fs::FS &fs, const char *, const char *);
