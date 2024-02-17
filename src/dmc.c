#include <stdio.h>
#include <string.h>

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

int main(int argc, char *argv[])
{
    char *filePath;
    char *encoding;

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

    // Si llegamos aquí, significa que no hubo error
    // Continúa con la ejecución del programa
    // ...

    return 0; // Finaliza con éxito
}