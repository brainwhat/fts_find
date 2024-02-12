#include <stdio.h>  // Стандартные библиотеки
#include <stdlib.h>
#include <fts.h>    // fts_open(), fts_read()
#include <getopt.h> // getopt_long()
#include <errno.h>  // Обработка ошибок
#include <string.h> // strcmp()

char *debug = NULL;
int cnt = 0;

// Функция поиска находится в файле walk_func.c
void walk_func(char **dirs, char *msg)
{
    // Открываем директорию с параметром
    // FTS_PHYSICAL - с игнорированием символьных ссылок
	FTS *fts_h = fts_open(dirs, FTS_PHYSICAL, NULL);

	if (!fts_h)
	{
		// При невозможности открыти выводим ошибку
		perror("fts_open failed");
		return;
	}
	while(1)
	{
		errno = 0;
        // Начинаем открывать файлы
		FTSENT *ent = fts_read(fts_h);
		if (ent == NULL)
		{
            // Если файл поврежден, не открывается, выводим ошибку
			if (errno != 0)
			{
				perror("fts_read() error!");
			}
            // В директории больше нет файлов
			else
				break;
		}
		if (ent->fts_info == FTS_F)
		{ 
            // entry - обычный файл -> читаем
			if (debug)
            {
                fprintf(stderr, "Debug: Checking file: %s\n", ent->fts_name);
            }
            // Длина искомой последовательности
			size_t len = strlen(msg);
            // Выделяем память
			char *buffer = malloc(sizeof(char) * len);
            
            // Открываем файл для чтения в бинарном формате
			FILE *fd = fopen(ent->fts_name, "rb");
			if (!fd)
			{ 
                // Не открывается -> выводим ошибку
                // И продолжаем итерацию
				perror("fopen() failed!");
				continue;
			}
            // Считываем первые len байтов
			size_t tmp = fread(buffer, sizeof(char), len, fd);
            
			if (tmp < len)
			{
				// Если дошли до конца файла -
                // Значит в файле < len байтов
				if (feof(fd))
					fprintf(stderr, "Error in %s: File too small!\n", ent->fts_path);
				// Если не дошли до конца -> ошибка
				else
					perror("fread() error:");
                // Переход к следующему файлу
				continue;
			}
			if (memcmp(msg, buffer, len) == 0)
			{ // Нашли совпадение -> выводим имя файла.
              // strcmp возвращает 0,
              // если строки совпадают

				printf("Found file! %s\n", ent->fts_path);
                // Заркываем файл, освобождаем буфер
				free(buffer);
				fclose(fd);
				cnt++;
                // Продолжаем итерацию
				continue;
			}
			else
				while (!feof(fd))
				{	// Пока файл не закончится
                    // считываем по одному элементу
					char t[1];
					tmp = fread(t, sizeof(char), 1, fd);
					if(tmp == 0 && !feof(fd)) {
					perror("fread error:");
					break;
					}
                    // Сдвигаем все элементы массива влево
					for (size_t i = 0; i < len - 1; i++)
					{
						buffer[i] = buffer[i + 1];
					}
                    // А в конец ставим новый
					buffer[len - 1] = t[0];
					if (memcmp(msg, buffer, len) == 0)
					{
                        // Нашли последовательность -> выводим
						printf("Found file! %s\n", ent->fts_path);
						cnt++;
						break;
						;
					}
				}
            // Не забываем закрыть файл
			fclose(fd);
            // И освободить память
			free(buffer);
		}
	}
	fts_close(fts_h);
}

// Функции вывода помощи и версии
// Находятся в файле funcs.c
void print_usage();
void print_version();

int main(int argc, char **argv)
{
    // Проверяем число аргументов и выводим ошибку в случае <2
    if (argc < 2) {
            fprintf(stderr, "Error: Insufficient arguments.\n");
            print_usage();
            return 1;
        }
        // При 2 аргументах выводим помощь/версию
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage();
            return 0;
        }

        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            print_version();
            return 0;
        }
	
	if (argc != 3)
	{
		// При недостатке подходящих аргументов выводим ошибку
		printf("Usage: %s [dir] 0xhh[hh*]\n", argv[0]);
		exit(1);
	}
	debug = getenv("LAB11DEBUG");
	char *msg;
    
    // Последовательность байтов должна начинаться на 0x
	if (argv[argc - 1][0] != '0' || argv[argc - 1][1] != 'x')
	{
		printf("Form of last argument: '0xhh[hh*]\n");
		exit(1);
	}
	else
	{
        // Выделяем память для последовательности байтов
        // в 2 раза меньше, т.к. hh занимает 1 байт
        // и в конце должен стоять '\0'
		size_t len = strlen(argv[argc - 1]);
		msg = malloc(sizeof(char) * len / 2);

        // Считываем по две hex цифры, переводим их в char
        // и сохраняем в msg
		for (size_t i = 1; i < len / 2; i++)
		{
			size_t t = sscanf(argv[argc - 1] + (2 * i), "%2hhx", msg + i - 1);
			if (t != 1)
				perror("sscanf() error!");
		}
        // Ставим '\0' для конца строки
		msg[len / 2 - 1] = '\0';
	}

    // Вызываем функцию поиска
	walk_func(&argv[argc - 2], msg);
	free(msg); // тк выделяли память - освобождаем
	printf("%d files found\n", cnt);
	return 0;
}