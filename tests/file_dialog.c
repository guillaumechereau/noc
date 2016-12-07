
#define NOC_FILE_DIALOG_IMPLEMENTATION
#define NOC_FILE_DIALOG_GTK

#include "noc_file_dialog.h"

int main()
{
    const char *ret;
    // Open a png file
    printf("Open dialog for an image file\n");
    ret = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN,
                               "png\0*.png\0jpg\0*.jpg;*.jpeg\0", NULL, NULL);
    printf("Save dialog for the same file\n");
    ret = noc_file_dialog_open(NOC_FILE_DIALOG_SAVE,
                               "png\0*.png\0", ret, NULL);

    printf("Open dialog for a directory\n");
    ret = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN | NOC_FILE_DIALOG_DIR,
                               NULL, NULL, NULL);

    return 0;
}
