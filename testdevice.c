#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd;
    char write_buffer[] = "Hello, World!";
    char read_buffer[80];

    // 打开设备文件
    fd = open("/dev/mychardev", O_RDWR);
    if (fd < 0) {
        perror("Cannot open device file");
        return 1;
    }

    // 写入数据
    if (write(fd, write_buffer, strlen(write_buffer)) != (ssize_t)strlen(write_buffer)) {
        perror("Write failed");
        close(fd);
        return 1;
    }

    // 重置文件指针
    lseek(fd, 0, SEEK_SET);

    // 读取数据
    ssize_t bytes_read = read(fd, read_buffer, 80);
    if (bytes_read < 0) {
        perror("Read failed");
        close(fd);
        return 1;
    }
    read_buffer[bytes_read] = '\0';  // 确保字符串以null结尾
    printf("Read from device: %s\n", read_buffer);

    // 关闭设备文件
    close(fd);
    return 0;
}