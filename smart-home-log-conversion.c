#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlschemastypes.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define MAX_LOC_LEN 31
#define MAX_FIRM_LEN 10
#define MAX_LINE_LEN 256
#define MAX_ENTRIES 1000

typedef struct {
    char device_id[8];
    char timestamp[20];
    float temperature;
    int humidity;
    char status[4];
    char location[MAX_LOC_LEN];
    char alert_level[10];
    int battery;
    char firmware_ver[MAX_FIRM_LEN];
    int event_code;
} LogEntry;

    typedef struct {
    int keyStart;
    int keyEnd;
    char order[5];
    } SortParams;

SortParams sortParams;

int read_sort_params(const char *filename, SortParams *params) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("setupParams.json not opening");
        return 0;
    }

    char buffer[256];
    fread(buffer, sizeof(char), sizeof(buffer) - 1, fp);
    fclose(fp);

    sscanf(buffer,
           " { \"dataFileName\" : \"logdata.dat\" , \"keyStart\" : %d , \"keyEnd\" : %d , \"order\" : \"%4s\" }",
           &params->keyStart, &params->keyEnd, params->order);

    return 1;
}

int compare_entries(const void *a, const void *b) {
    const LogEntry *entryA = (const LogEntry *)a;
    const LogEntry *entryB = (const LogEntry *)b;

    char keyA[16], keyB[16];
    strncpy(keyA, entryA->device_id + sortParams.keyStart, sortParams.keyEnd - sortParams.keyStart + 1);
    keyA[sortParams.keyEnd - sortParams.keyStart + 1] = '\0';

    strncpy(keyB, entryB->device_id + sortParams.keyStart, sortParams.keyEnd - sortParams.keyStart + 1);
    keyB[sortParams.keyEnd - sortParams.keyStart + 1] = '\0';

    int cmp = strcmp(keyA, keyB);
    return strcmp(sortParams.order, "ascii") == 0 ? cmp : -cmp;
}

void ascii(char *status) {
    int j = 0;
    for (int i = 0; status[i] != '\0'; i++) {
        if ((unsigned char)status[i] <= 127) {
            status[j++] = status[i];
        }
    }
    status[j] = '\0';
}



void emojis(char *status) {
    if (strstr(status, "⚠") != NULL) {
        strcpy(status, "WARN");
    } else if (strstr(status, "✓") != NULL) {
        strcpy(status, "OK");
    } else if (strstr(status, "❌") != NULL) {
        strcpy(status, "BAD");
    }
}




void read_csv_file(const char *filename, char separator, LogEntry *entries, int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("CSV no open");
        exit(1);
    }

    char line[MAX_LINE_LEN];
    int i = 0;

   
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && i < MAX_ENTRIES) {
        line[strcspn(line, "\r\n")] = 0;

        char *token = strtok(line, &separator);
        if (token && strlen(token) > 0)
            strncpy(entries[i].device_id, token, sizeof(entries[i].device_id) - 1);
        else
            strcpy(entries[i].device_id, ""); 

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0)
            strncpy(entries[i].timestamp, token, sizeof(entries[i].timestamp) - 1);
        else
            strcpy(entries[i].timestamp, "");

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0)
            entries[i].temperature = atof(token);
        else
            entries[i].temperature = 0.0;

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0)
            entries[i].humidity = atoi(token);
        else
            entries[i].humidity = 0;

        
        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0) {
            strncpy(entries[i].status, token, sizeof(entries[i].status) - 1);
            emojis(entries[i].status);
            ascii(entries[i].status);
            if (entries[i].status[0] == '\0') {
                strcpy(entries[i].status, "UNKNOWN");
            }
        } else {
            strcpy(entries[i].status, "UNKNOWN");
        }

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0) {
            strncpy(entries[i].location, token, sizeof(entries[i].location) - 1);
            ascii(entries[i].location);
        } else {
            strcpy(entries[i].location, ""); 
        }

        
        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0)
            strncpy(entries[i].alert_level, token, sizeof(entries[i].alert_level) - 1);
        else
            strcpy(entries[i].alert_level, "NONE");

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0)
            entries[i].battery = atoi(token);
        else
            entries[i].battery = 0;  

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0) {
            strncpy(entries[i].firmware_ver, token, sizeof(entries[i].firmware_ver) - 1);
            ascii(entries[i].firmware_ver);
        } else {
            strcpy(entries[i].firmware_ver, ""); 
        }

        token = strtok(NULL, &separator);
        if (token && strlen(token) > 0)
            entries[i].event_code = atoi(token);
        else
            entries[i].event_code = 0;

        i++;
    }

    *count = i;
    fclose(fp);
}





void write_binary_file(const char *filename, LogEntry *entries, int count) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Binary file isn't writeable");
        exit(1);
    }

    size_t written = fwrite(entries, sizeof(LogEntry), count, fp);
    if (written != count) {
        printf("Log count not same as counted! Expected: %d, Writen: %zu\n", count, written);
    } else {
        printf("All data writen in %s  file.\n", filename);
    }

    fclose(fp);
}

void to_hex_big_endian(int value, char *output) {
    sprintf(output, "%08X", value);
}

void to_hex_little_endian(int value, char *output) {
    unsigned char bytes[4];
    bytes[0] = (value >> 0) & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
    sprintf(output, "%02X%02X%02X%02X", bytes[0], bytes[1], bytes[2], bytes[3]);
}

void write_xml_file(const char *filename, LogEntry *entries, int count, const char *newline) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("XML can't be writen");
        exit(1);
    }
//encoding
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>%s", newline);
    fprintf(fp, "<smartlogs>%s", newline);  

    for (int i = 0; i < count; i++) {
        fprintf(fp, "  <entry id=\"%d\">%s", i + 1, newline);
        fprintf(fp, "    <device>%s", newline);
        fprintf(fp, "      <device_id>%s</device_id>%s", entries[i].device_id, newline);
        fprintf(fp, "      <location>%s</location>%s", entries[i].location, newline);
        fprintf(fp, "      <firmware_ver>%s</firmware_ver>%s", entries[i].firmware_ver, newline);
        fprintf(fp, "    </device>%s", newline);

        fprintf(fp, "    <metrics status=\"%s\" alert_level=\"%s\">%s",
                entries[i].status, entries[i].alert_level, newline);
        fprintf(fp, "      <temperature>%.1f</temperature>%s", entries[i].temperature, newline);
        fprintf(fp, "      <humidity>%d</humidity>%s", entries[i].humidity, newline);
        fprintf(fp, "      <battery>%d</battery>%s", entries[i].battery, newline);
        fprintf(fp, "    </metrics>%s", newline);

        fprintf(fp, "    <timestamp>%s</timestamp>%s", entries[i].timestamp, newline);

        char hexBig[9], hexLittle[9];
        to_hex_big_endian(entries[i].event_code, hexBig);
        to_hex_little_endian(entries[i].event_code, hexLittle);
        fprintf(fp, "    <event_code hexBig=\"%s\" hexLittle=\"%s\" decimal=\"%d\">%d</event_code>%s",
                hexBig, hexLittle, entries[i].event_code, entries[i].event_code, newline);

        fprintf(fp, "  </entry>%s", newline);
    }

    fprintf(fp, "</smartlogs>%s", newline);  
    fclose(fp);
    printf("XML complete.\n");
}



void validate_xml(const char *xmlFile, const char *xsdFile) {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr parser_ctxt;
    xmlSchemaValidCtxtPtr valid_ctxt;

    xmlLineNumbersDefault(1);

    parser_ctxt = xmlSchemaNewParserCtxt(xsdFile);
    if (!parser_ctxt) {
        fprintf(stderr, "XSD couldn't be read: %s\n", xsdFile);
        return;
    }

    schema = xmlSchemaParse(parser_ctxt);
    xmlSchemaFreeParserCtxt(parser_ctxt);

    if (!schema) {
        fprintf(stderr, "XSD scheme unvalid.\n");
        return;
    }

    doc = xmlReadFile(xmlFile, NULL, 0);
    if (!doc) {
        fprintf(stderr, "XML couln't be read: %s\n", xmlFile);
        xmlSchemaFree(schema);
        return;
    }

    valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (!valid_ctxt) {
        fprintf(stderr, "Error.\n");
        xmlFreeDoc(doc);
        xmlSchemaFree(schema);
        return;
    }

    int ret = xmlSchemaValidateDoc(valid_ctxt, doc);

    if (ret == 0) {
        printf("XML Validation succes: %s xsd.\n", xmlFile);
    } else if (ret > 0) {
        printf("XML unvalidated: %s XSD not.\n", xmlFile);
    } else {
        printf("Error within.\n");
    }

    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlFreeDoc(doc);
    xmlSchemaFree(schema);
    xmlCleanupParser();
}

void print_usage() {
    printf("Usage:\n");
    printf("./deviceTool <input_file> <output_file> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-h]\n\n");
    printf("<conversion_type>:\n");
    printf("1 - CSV to Binary\n");
    printf("2 - Binary to XML\n");
    printf("3 - XML Validation with XSD\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("USAGE: %s <input_file> <output_file> [-separator <1|2|3>] [-opsys <1|2|3>]\n", argv[0]);
        printf("  -separator: 1 = ',' | 2 = tab | 3 = ';'\n");
        printf("  -opsys    : 1 = Windows | 2 = Linux | 3 = macOS\n");
        return 1;
    }

    char *input_file = argv[1];
    char *output_file = argv[2];
    //default (windows)
    int separatorCode = 1; 
    int opsys = 1;         

    // other inputs
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-separator") == 0 && i + 1 < argc) {
            separatorCode = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-opsys") == 0 && i + 1 < argc) {
            opsys = atoi(argv[i + 1]);
        }
    }

    char separator = ',';
    char newline[3] = "\r\n"; 

    if (separatorCode == 2) separator = '\t';
    else if (separatorCode == 3) separator = ';';

    if (opsys == 2) strcpy(newline, "\n"); // for Linux
    else if (opsys == 3) strcpy(newline, "\r"); // for MacOS

    int choice = 0;

    // Start menu
    printf("\nWelcome to Smart Logs Processor!\n");
    printf("Please select an operation:\n");
    printf("1. Convert CSV to Binary\n");
    printf("2. Convert Binary to XML\n");
    printf("3. Validate XML with XSD\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
    printf("For example: ./deviceTool logdata.xml logdata.xsd 3 -separator 1 -opsys 2\n");

    scanf("%d", &choice);

    if (choice == 1) {
        LogEntry entries[MAX_ENTRIES];
        int entry_count = 0;

        read_csv_file(input_file, separator, entries, &entry_count);
        write_binary_file(output_file, entries, entry_count);

        printf("%d number of logs transformed from CSV to Binary.\n", entry_count);

    } else if (choice == 2) {
        FILE *fp = fopen(input_file, "rb");
        if (!fp) {
            perror("Binary couldn't read");
            return 1;
        }

        LogEntry loaded_entries[MAX_ENTRIES];
        int loaded_count = fread(loaded_entries, sizeof(LogEntry), MAX_ENTRIES, fp);
        fclose(fp);

        SortParams params;
        if (read_sort_params("setupParams.json", &params)) {
            sortParams = params;
            qsort(loaded_entries, loaded_count, sizeof(LogEntry), compare_entries);
            printf("Logs %s sorted: device_id[%d-%d]\n", params.order, params.keyStart, params.keyEnd);
        }

        write_xml_file(output_file, loaded_entries, loaded_count, newline);

    } else if (choice == 3) {
        validate_xml(input_file, output_file);

    } else if (choice == 4) {
        printf("Error.\n");
        return 0;

    } else {
        printf("Wrong choice!");
        return 1;
    }

    return 0;
}

// Utku Taha Polat & İlayda Meral
// 2023510176 & 2023510169