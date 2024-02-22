#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/userInterface.h"
#include "../include/selectView.h"
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <limits.h>
#include <libgen.h>
#define BUFFER_SIZE 1024
// Array de formatos válidos
const char *validFormats[] = {"mp3", "wav", "aac", "flac", "ogg"};
const int validFormatsCount = sizeof(validFormats) / sizeof(validFormats[0]);
UserInterface ui;

typedef struct
{
    char *fileFormat;
    char *audioBuffer;
    double fileSize;
} ConvertedAudioBuffer;

void listConvertedFiles(const char *tempDir)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileInfo;
    char filePath[PATH_MAX];
    int i = 0; // Iniciar contador en 0

    if ((dir = opendir(tempDir)) == NULL)
    {
        perror("opendir() error");
        return;
    }

    printf("Archivos convertidos disponibles:\n");
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // Si es un archivo regular
            snprintf(filePath, sizeof(filePath), "%s/%s", tempDir, entry->d_name);
            if (stat(filePath, &fileInfo) == 0)
            {
                double sizeInMb = fileInfo.st_size / (1024.0 * 1024.0);
                printf("[%d] (%.2fMB) %s\n", i++, sizeInMb, filePath);
            }
        }
    }
    closedir(dir);
}

int outputErrorMessage(const char *message)
{
    // Imprime un mensaje de error en stderr
    fprintf(stderr, "%s\n", message);
    return 1; // Devuelve un código de error
}

// Función para verificar si el encoding es válido
int isValidEncoding(const char *encoding)
{
    for (int i = 0; i < validFormatsCount; i++)
    {
        if (strcmp(encoding, validFormats[i]) == 0)
        {
            return 1; // Encoding válido
        }
    }
    return 0; // Encoding no válido
}

int checkComingArgs(int argc, char *argv[], char **filePath, char **encoding)
{

    int fFlag = 0, eFlag = 0; // Flags para detectar -f y -e

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {

            if (i + 1 < argc)
            {
                *filePath = argv[++i]; // Asume que el siguiente argumento es el valor de FILE por eso el ++i
                fFlag = 1;             // Marca que se encontró -f
            }
            else
            {
                outputErrorMessage("Error: Falta el valor para -f o el valor es inválido.\n");
                return 1;
            }
        }
        else if (strncmp(argv[i], "-e=", 3) == 0)
        {
            *encoding = argv[i] + 3; // Obtiene el valor de ENCODING
            if (strlen(*encoding) == 0)
            { // Verifica que el valor de encoding no esté vacío
                outputErrorMessage("Error: Falta el valor para -e.\n");
                return 1;
            }
            eFlag = 1; // Marca que se encontró -e
        }
        else
        {
            fprintf(stderr, "Opción no reconocida: %s\n", argv[i]);
            return 1;
        }
    }

    if (!fFlag)
    { // Verifica si -f y FILE están presentes
        outputErrorMessage("Uso: dmc [OPCIONES] -f FILE\n");
        return 1;
    }

    if (eFlag && !isValidEncoding(*encoding))
    { // Verifica si el encoding es válido
        outputErrorMessage("Encoding no válido. Los formatos válidos son: mp3, wav, aac, flac, ogg\n");
        return 1;
    }

    // Aquí, la lógica para manejar los argumentos válidos

    return 0;
}

int isAiffFile(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot)
    {
        return 0; // No hay extensión de archivo
    }
    return strcmp(dot, ".aif") == 0 || strcmp(dot, ".aiff") == 0;
}

int containsAiffFiles(const char *directoryPath)
{
    struct dirent *entry;
    DIR *dp = opendir(directoryPath);
    if (dp == NULL)
    {
        return 0;
    }

    int found = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // Es un archivo regular
            if (isAiffFile(entry->d_name))
            {
                found = 1; // Se encontró al menos un archivo AIFF
                break;
            }
        }
    }

    closedir(dp);
    return found;
}

int processDirectory(const char *directoryPath, char ***filesList, int *listSize)
{
    DIR *dir;
    struct dirent *entry;
    char **list = NULL;
    int size = 0;

    if (!(dir = opendir(directoryPath)))
        return 0; // Directory cannot be opened

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG && isAiffFile(entry->d_name))
        {
            // Resize the list to hold another filename
            char **temp = realloc(list, sizeof(char *) * (size + 1));
            if (!temp)
            {
                // Handle allocation error
                closedir(dir);
                free(list); // Remember to free the previously allocated memory
                return 0;
            }
            list = temp;
            list[size] = strdup(entry->d_name); // Duplicate the filename
            if (!list[size])
            {
                // Handle allocation error
                closedir(dir);
                for (int i = 0; i < size; ++i)
                    free(list[i]);
                free(list);
                return 0;
            }
            ++size;
        }
    }
    closedir(dir);

    *filesList = list;
    *listSize = size;
    return 1; // Success
}

int convert_to_format(const char *input_file, const char *output_format, char **audioBuffer, double *fileSize)
{
    char output_file[1024];
    char command[1024];

    // Hacemos una copia del input_file para usar con basename, ya que puede modificar la entrada
    char *input_copy = strdup(input_file);
    if (!input_copy)
    {
        perror("Error al duplicar el nombre del archivo");
        return 1;
    }

    // Obtiene el nombre base del archivo (sin la ruta)
    char *base = basename(input_copy); // basename() podría alterar input_copy

    // Encuentra y elimina la extensión .aif o .aiff
    char *dot = strrchr(base, '.');
    if (dot && (strcmp(dot, ".aif") == 0 || strcmp(dot, ".aiff") == 0))
    {
        *dot = '\0'; // Corta la cadena en el punto, eliminando la extensión
    }

    if (audioBuffer != NULL)
    {
        output_file[0] = '-';
    }
    else
    {
        snprintf(output_file, 1024, "%s.%s", input_copy, output_format);
    }

    free(input_copy); // Libera la memoria de la copia del nombre del archivo

    if (strcmp(output_format, "aac") == 0)
    {
        snprintf(command, sizeof(command), "ffmpeg -loglevel 0 -y -i '%s' -f adts %s", input_file, output_file);
    }
    else
    {
        snprintf(command, sizeof(command), "ffmpeg -loglevel 0 -y -i '%s' -f %s \"%s\"", input_file, output_format, output_file);
    }

    if (audioBuffer != NULL)
    {
        FILE *pipe = popen(command, "r");

        if (pipe == NULL)
        {
            perror("popen");
            return 1;
        }

        size_t bytesWritten;
        char buffer[1024];
        int result_len = 0;

        // Read the output into the buffer
        while ((bytesWritten = fread(buffer, 1, BUFFER_SIZE, pipe)) > 0)
        {
            size_t len = bytesWritten;

            char *str = realloc(*audioBuffer, result_len + len);
            if (!str)
            {
                free(audioBuffer);
                return 1;
            }
            *audioBuffer = str;
            memcpy(*audioBuffer + result_len, buffer, bytesWritten);
            result_len += len;
        }

        *fileSize = (double)result_len;
    }
    else
    {
        int result = system(command);
        if (result != 0)
        {
            fprintf(stderr, "Error al convertir el archivo a formato %s.\n", output_format);
        }
        else
        {
            printf("Conversión completada: %s\n", output_file);
        }
    }

    return 0;
}

// Definición de estructura para pasar datos al hilo
struct ThreadData
{
    char *inputFilePath;
    char *outputFormat;
    // Inputs opcionales
    char **audioBuffer;
    double *fileSize;
};

// Función que será ejecutada por cada hilo
void *threadConvert(void *arg)
{

    struct ThreadData *data = (struct ThreadData *)arg;
    convert_to_format(data->inputFilePath, data->outputFormat, data->audioBuffer, data->fileSize);
    free(data->inputFilePath);
    free(data->outputFormat);
    pthread_exit(NULL);
}

void process_files_parallel(const char *directoryPath, const char *encoding, char **filesList, int filesCount)
{
    pthread_t threads[filesCount];
    int i;

    // Iterar sobre la lista de archivos y crear un hilo para cada uno
    for (i = 0; i < filesCount; i++)
    {
        struct ThreadData *data = malloc(sizeof(struct ThreadData));
        if (!data)
        {
            fprintf(stderr, "Error de asignación de memoria.\n");
            exit(1);
        }
        // Calcular el tamaño necesario para la ruta completa
        char *fullPath = malloc(PATH_MAX);
        if (!fullPath)
        {
            fprintf(stderr, "Error de asignación de memoria.\n");
            exit(1);
        }

        // Construir la ruta completa
        snprintf(fullPath, PATH_MAX, "%s/%s", directoryPath, filesList[i]);

        data->inputFilePath = fullPath; // Usar la ruta completa strdup(filesList[i]); // Copiar la ruta del archivo
        if (!data->inputFilePath)
        {
            fprintf(stderr, "Error de asignación de memoria.\n");
            exit(1);
        }
        data->outputFormat = strdup(encoding); // Copiar el formato de salida
        if (!data->outputFormat)
        {
            fprintf(stderr, "Error de asignación de memoria.\n");
            exit(1);
        }

        data->audioBuffer = NULL;
        data->fileSize = NULL;
        // Crear un hilo para convertir el archivo
        if (pthread_create(&threads[i], NULL, threadConvert, (void *)data) != 0)
        {
            fprintf(stderr, "Error al crear hilo.\n");
            exit(1);
        }
    }

    // Esperar a que todos los hilos terminen
    for (i = 0; i < filesCount; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void process_files(const char *directoryPath, const char *encoding, char **filesList, int filesCount)
{
    char inputFilePath[512];
    char outputFilePath[512];

    // Verificar si hay archivos AIFF para procesar
    if (filesCount == 0)
    {
        fprintf(stderr, "No hay archivos AIFF para procesar en el directorio especificado.\n");
        return;
    }

    // Llamar a la función para procesar los archivos en paralelo
    process_files_parallel(directoryPath, encoding, filesList, filesCount);
}

int main(int argc, char *argv[])
{
    char *filePath = NULL;
    char *encoding = NULL;
    char **aiffFilesList;
    int aiffFilesCount;

    // Verifica los argumentos y obtiene filePath y encoding si están presentes
    if (checkComingArgs(argc, argv, &filePath, &encoding) == 0)
    {

        // No hubo error, el programa puede continuar
        printf("Archivo a procesar: %s\n", filePath);
        if (encoding != NULL)
        {
            printf("Encoding especificado: %s\n", encoding);
        }
    }
    else
    {
        // Hubo un error, termina la ejecución
        return 1; // Código de error
    }

    struct stat path_stat;
    if (stat(filePath, &path_stat) != 0)
    {
        perror("Error al acceder a la ruta especificada");
        return 1;
    }

    if (S_ISREG(path_stat.st_mode))
    { // Es un archivo
        if (isAiffFile(filePath))
        {
            if (encoding != NULL)
            {
                printf("Advertencia: El parámetro -e= no es necesario para la conversión de un único archivo AIFF.\n");
            }

            pthread_t threads[validFormatsCount]; // Creamos un arreglo de hilos
            ConvertedAudioBuffer convertedAudios[validFormatsCount];
            SelectView *fileFormatSelectView;

            // Creamos un hilo para cada formato y realizamos la conversión en paralelo
            for (int i = 0; i < validFormatsCount; ++i)
            {
                // Creamos una estructura ThreadData para pasar los datos al hilo
                struct ThreadData *data = malloc(sizeof(struct ThreadData));
                if (!data)
                {
                    fprintf(stderr, "Error de asignación de memoria.\n");
                    return 1;
                }
                // Configuramos los datos para este hilo
                data->inputFilePath = strdup(filePath);
                if (!data->inputFilePath)
                {
                    fprintf(stderr, "Error de asignación de memoria.\n");
                    free(data);
                    return 1;
                }
                data->outputFormat = strdup(validFormats[i]);
                if (!data->outputFormat)
                {
                    fprintf(stderr, "Error de asignación de memoria.\n");
                    free(data->inputFilePath);
                    free(data);
                    return 1;
                }

                convertedAudios[i].audioBuffer = NULL;
                convertedAudios[i].fileFormat = strdup(validFormats[i]);
                data->audioBuffer = &convertedAudios[i].audioBuffer;
                data->fileSize = &convertedAudios[i].fileSize;

                // Creamos el hilo
                if (pthread_create(&threads[i], NULL, threadConvert, (void *)data) != 0)
                {
                    fprintf(stderr, "Error al crear hilo.\n");
                    free(data->inputFilePath);
                    free(data->outputFormat);
                    free(data);
                    return 1;
                }
            }

            // Esperamos a que todos los hilos terminen
            for (int i = 0; i < validFormatsCount; ++i)
            {
                pthread_join(threads[i], NULL);
            }

            char *formatSelections[validFormatsCount];

            for (int i = 0; i < validFormatsCount; ++i)
            {
                formatSelections[i] = malloc(sizeof(char) * 30);
                snprintf(formatSelections[i], 30, "%s) %.5f MB\n", convertedAudios[i].fileFormat, convertedAudios[i].fileSize / (1024.0 * 1024.0));
            }

            fileFormatSelectView = createSelectView("Selecciona uno de los formatos", formatSelections, validFormatsCount, "Escoge un numero:");

            int selectedFileFormat = enableSelectView(fileFormatSelectView) - 1;

            FILE *outputFile;
            char outputFileName[1024];
            char *dot = strrchr(filePath, '.');
            *dot = '\0';

            snprintf(outputFileName, 1024, "%s.%s", filePath, convertedAudios[selectedFileFormat].fileFormat);

            outputFile = fopen(outputFileName, "wb");

            if (outputFile == NULL)
            {
                perror("fopen");
                return 1;
            }
            fwrite(convertedAudios[selectedFileFormat].audioBuffer, 1, convertedAudios[selectedFileFormat].fileSize, outputFile);
            printf("\ndone %s with option %d\n,", outputFileName, selectedFileFormat);
        }
        else
        {
            fprintf(stderr, "El archivo proporcionado no es un archivo AIFF.\n");
            return 1;
        }
    }
    else if (S_ISDIR(path_stat.st_mode))
    { // Es un directorio
        if (encoding == NULL)
        {
            fprintf(stderr, "Error: Se debe especificar un formato de encoding con la opción -e= cuando se procesa un directorio.\n");
            return 1;
        }
        else if (!containsAiffFiles(filePath))
        {
            fprintf(stderr, "Error: No se encontraron archivos AIFF en el directorio especificado.\n");
            return 1;
        }
        else
        {
            if (!processDirectory(filePath, &aiffFilesList, &aiffFilesCount))
            {
                fprintf(stderr, "Error procesando el directorio.\n");
                return 1;
            }
            printf("Se encontraron %d archivos AIFF para procesar.\n", aiffFilesCount);

            // process_directory(filePath, encoding);
            process_files(filePath, encoding, aiffFilesList, aiffFilesCount);
        }
    }
    else
    {
        fprintf(stderr, "La ruta proporcionada no es un archivo ni un directorio.\n");
        return 1;
    }

    // ... (El resto de la lógica de tu programa)

    return 0;
}
