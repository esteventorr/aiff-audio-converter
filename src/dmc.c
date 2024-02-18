#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

// Array de formatos válidos
const char *validFormats[] = {"mp3", "wav", "aac", "flac", "ogg"};
const int validFormatsCount = sizeof(validFormats) / sizeof(validFormats[0]);

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
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                *filePath = argv[++i]; // Asume que el siguiente argumento es el valor de FILE
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
    printf("Archivo: %s\n", *filePath);
    if (eFlag)
    {
        printf("Encoding: %s\n", *encoding);
    }

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

void convert_to_format(const char *input_file, const char *output_format)
{
    char output_file[512];
    char base_name[512];
    char *dot;

    strncpy(base_name, input_file, sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0'; // Asegura que la cadena esté terminada

    dot = strrchr(base_name, '.');
    if (dot && (strcmp(dot, ".aif") == 0 || strcmp(dot, ".aiff") == 0))
    {
        *dot = '\0';
    }

    // Asegurarse de que la combinación de base_name y output_format no exceda el límite
    size_t total_length = strlen(base_name) + strlen(output_format) + 1; // +1 por el punto
    if (total_length >= sizeof(output_file) - 1)
    {
        fprintf(stderr, "El nombre del archivo de salida es demasiado largo.\n");
        return;
    }

    snprintf(output_file, sizeof(output_file), "%s.%s", base_name, output_format);

    char command[1024];

    printf("Convirtiendo %s a formato %s...\n", input_file, output_format);

    // Selecciona los parámetros de conversión basados en el formato de salida
    if (strcmp(output_format, "mp3") == 0)
    {
        // Para MP3, especificamos la tasa de bits
        snprintf(command, sizeof(command), "ffmpeg -i '%s' -codec:a libmp3lame -b:a 192k '%s'", input_file, output_file);
    }
    else if (strcmp(output_format, "wav") == 0)
    {
        // WAV no necesita codecs especiales, pero podríamos especificar opciones si fuera necesario
        snprintf(command, sizeof(command), "ffmpeg -i '%s' '%s'", input_file, output_file);
    }
    else if (strcmp(output_format, "aac") == 0)
    {
        // Para AAC, especificamos el codec
        snprintf(command, sizeof(command), "ffmpeg -i '%s' -codec:a aac -b:a 128k '%s'", input_file, output_file);
    }
    else if (strcmp(output_format, "flac") == 0)
    {
        // FLAC puede especificar la calidad de compresión, pero usamos la configuración predeterminada aquí
        snprintf(command, sizeof(command), "ffmpeg -i '%s' -codec:a flac '%s'", input_file, output_file);
    }
    else if (strcmp(output_format, "ogg") == 0)
    {
        // Para OGG, especificamos el codec Vorbis
        snprintf(command, sizeof(command), "ffmpeg -i '%s' -codec:a libvorbis -qscale:a 3 '%s'", input_file, output_file);
    }
    else
    {
        // Si el formato no es reconocido, imprime un error y sale
        fprintf(stderr, "Formato de salida no reconocido: %s\n", output_format);
        return;
    }

    // Ejecuta el comando FFmpeg
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

void process_files(const char *directoryPath, const char *encoding, char **filesList, int filesCount)
{
    char inputFilePath[512];
    char outputFilePath[512];

    for (int i = 0; i < filesCount; i++)
    {
        // Construye la ruta completa del archivo de entrada
        snprintf(inputFilePath, sizeof(inputFilePath), "%s/%s", directoryPath, filesList[i]);

        // Construye la ruta completa del archivo de salida
        // Primero, quita la extensión .aif/.aiff y luego añade el formato de salida
        strncpy(outputFilePath, inputFilePath, sizeof(outputFilePath) - 1);
        outputFilePath[sizeof(outputFilePath) - 1] = '\0'; // Asegúrate de que la cadena esté terminada
        char *dot = strrchr(outputFilePath, '.');
        if (dot)
            *dot = '\0'; // Elimina la extensión
        strcat(outputFilePath, ".");
        strcat(outputFilePath, encoding); // Añade la nueva extensión

        // Llama a convert_to_format para este archivo
        convert_to_format(inputFilePath, encoding); // Ajusta según sea necesario
    }
}

int main(int argc, char *argv[])
{
    char *filePath;
    char *encoding;
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
        // Aquí puedes continuar con la lógica de tu programa usando filePath y encoding
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
            // Procesar el archivo AIFF individual
            for (int i = 0; i < sizeof(validFormats) / sizeof(validFormats[0]); ++i)
            {
                convert_to_format(filePath, validFormats[i]);
                // Después de esta función, podrías obtener el tamaño del archivo de salida
                // y mostrarlo junto con el nombre del archivo al usuario.
            }
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
            // Aquí tienes un array 'aiffFilesList' con 'aiffFilesCount' elementos para procesar
            // Después de llamar a processDirectory
            printf("Archivos AIFF encontrados para procesar:\n");
            for (int i = 0; i < aiffFilesCount; ++i)
            {
                printf("%d: %s\n", i + 1, aiffFilesList[i]);
            }
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