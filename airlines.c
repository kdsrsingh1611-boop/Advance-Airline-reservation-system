#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#define MAX_SEATS 40

typedef struct Passenger {
    int id;
    char name[100];
    char gender;
    int age;
    char travelDate[20];
    char destination[100];
    char departureAirport[100];
    char seat[4];
    char travelClass[20];
    char arrivalTime[10];
    char departureTime[10];
    struct Passenger *next;
} Passenger;

Passenger *head = NULL;
int seatCount = 0;
char seatMap[MAX_SEATS][4];

void textToSpeech(const char *message) {
    char escaped[512];
    int j = 0;
    for (int i = 0; message[i] && j < sizeof(escaped) - 2; i++) {
        if (message[i] == '\'') {
            escaped[j++] = '\'';
            escaped[j++] = '\'';
        } else {
            escaped[j++] = message[i];
        }
    }
    escaped[j] = '\0';

    char command[1024];
    snprintf(command, sizeof(command),
        "PowerShell -Command \"Add-Type -AssemblyName System.Speech; "
        "$speak = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$speak.Speak('%s')\"", escaped);

    system(command);
}

void speechToText(char *buffer, size_t size) {
    while (1) {
        system("python speech_to_text.py");
        Sleep(1000);
        FILE *fp = fopen("speech_input.txt", "r");
        if (fp != NULL) {
            fgets(buffer, size, fp);
            buffer[strcspn(buffer, "\n")] = '\0';
            fclose(fp);
            if (strlen(buffer) > 0) break;
        }
        textToSpeech("I didn't catch that. Please speak again.");
    }
}

int speechToMenuChoice() {
    char buffer[100];
    speechToText(buffer, sizeof(buffer));
    printf("You said: %s\n", buffer); // debug
    for (int i = 0; buffer[i]; i++) buffer[i] = tolower(buffer[i]);

    if (strstr(buffer, "book") || strstr(buffer, "one") || strstr(buffer, "1")) return 1;
    if (strstr(buffer, "view") || strstr(buffer, "two") || strstr(buffer, "2")) return 2;
    if (strstr(buffer, "cancel") || strstr(buffer, "three") || strstr(buffer, "3")) return 3;
    if (strstr(buffer, "exit") || strstr(buffer, "four") || strstr(buffer, "4") || strstr(buffer, "quit")) return 4;

    return -1;
}

int generateID() { return 10000 + rand() % 90000; }

const char *randomAirport() {
    const char *airports[] = {
        "Indira Gandhi International Airport, Delhi",
        "Chhatrapati Shivaji Maharaj International Airport, Mumbai",
        "Kempegowda International Airport, Bangalore",
        "Chennai International Airport, Chennai",
        "Rajiv Gandhi International Airport, Hyderabad",
        "Netaji Subhas Chandra Bose International Airport, Kolkata"
    };
    int index = rand() % 6;
    return airports[index];
}

void randomTime(char *timeStr) {
    int hour = rand() % 24;
    int minute = rand() % 60;
    sprintf(timeStr, "%02d:%02d", hour, minute);
}

int validateName(const char *name) {
    int space = 0;
    for (int i = 0; name[i]; i++) {
        if (isdigit(name[i])) return 0;
        if (isspace(name[i])) space = 1;
    }
    return space;
}

void assignSeat(char seat[]) {
    for (int i = 0; i < MAX_SEATS; i++) {
        if (seatMap[i][0] == '\0') {
            sprintf(seatMap[i], "S%d", i + 1);
            strcpy(seat, seatMap[i]);
            return;
        }
    }
}

void bookTicket() {
    Passenger *newP = (Passenger *)malloc(sizeof(Passenger));
    newP->id = generateID();
    char input[100];

    textToSpeech("Enter your full name.");
    speechToText(input, sizeof(input));
    while (!validateName(input)) {
        textToSpeech("Invalid name. Please speak your full name again.");
        speechToText(input, sizeof(input));
    }
    strcpy(newP->name, input);
    printf("Name: %s\n", input);

    textToSpeech("Enter your gender. Say male or female.");
    speechToText(input, sizeof(input));
    newP->gender = (strstr(input, "female")) ? 'F' : 'M';

    textToSpeech("Enter your age.");
    speechToText(input, sizeof(input));
    newP->age = atoi(input);
    while (newP->age <= 0 || newP->age > 100) {
        textToSpeech("Invalid age. Please say a number between 1 and 100.");
        speechToText(input, sizeof(input));
        newP->age = atoi(input);
    }

    textToSpeech("Enter your date of travel.");
    speechToText(newP->travelDate, sizeof(newP->travelDate));

    textToSpeech("Enter your destination.");
    speechToText(newP->destination, sizeof(newP->destination));

    while (1) {
        textToSpeech("Select travel class. Say economy or business.");
        speechToText(input, sizeof(input));
        if (strstr(input, "business")) {
            strcpy(newP->travelClass, "Business");
            break;
        } else if (strstr(input, "economy")) {
            strcpy(newP->travelClass, "Economy");
            break;
        } else {
            textToSpeech("Please say economy or business.");
        }
    }

    strcpy(newP->departureAirport, randomAirport());
    randomTime(newP->arrivalTime);
    randomTime(newP->departureTime);
    assignSeat(newP->seat);

    newP->next = head;
    head = newP;
    seatCount++;

    char confirmMsg[100];
    sprintf(confirmMsg, "Ticket booked. ID %d, Seat %s", newP->id, newP->seat);
    textToSpeech(confirmMsg);
    printf("✅ Ticket Booked! ID: %d, Seat: %s\n", newP->id, newP->seat);
}

void displayPassengers() {
    Passenger *temp = head;
    if (!temp) {
        textToSpeech("No bookings found.");
        printf("No bookings found.\n");
        return;
    }

    printf("\n--- Booked Passengers ---\n");
    while (temp) {
        printf("ID: %d\nName: %s\nGender: %c\nAge: %d\nDestination: %s\nDate: %s\nSeat: %s\nClass: %s\nFrom: %s\nDeparture: %s | Arrival: %s\n\n",
               temp->id, temp->name, temp->gender, temp->age, temp->destination,
               temp->travelDate, temp->seat, temp->travelClass,
               temp->departureAirport, temp->departureTime, temp->arrivalTime);
        temp = temp->next;
    }
}

void saveToFile() {
    FILE *fp = fopen("passengers.txt", "w");
    Passenger *temp = head;
    while (temp) {
        fprintf(fp, "%d %s %c %d %s %s %s %s %s %s %s\n",
                temp->id, temp->name, temp->gender, temp->age,
                temp->travelDate, temp->destination, temp->departureAirport,
                temp->seat, temp->travelClass, temp->departureTime, temp->arrivalTime);
        temp = temp->next;
    }
    fclose(fp);
}

void cancelTicket() {
    if (!head) {
        textToSpeech("No bookings to cancel.");
        printf("No bookings to cancel.\n");
        return;
    }

    char input[100];
    textToSpeech("Speak the passenger ID to cancel.");
    speechToText(input, sizeof(input));
    int id = atoi(input);

    Passenger *temp = head, *prev = NULL;
    while (temp) {
        if (temp->id == id) {
            if (prev) prev->next = temp->next;
            else head = temp->next;

            for (int i = 0; i < MAX_SEATS; i++) {
                if (strcmp(seatMap[i], temp->seat) == 0) {
                    seatMap[i][0] = '\0';
                    break;
                }
            }

            char msg[100];
            sprintf(msg, "Ticket ID %d cancelled", id);
            textToSpeech(msg);
            printf("Ticket cancelled: %d\n", id);
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    textToSpeech("Passenger ID not found.");
    printf("Passenger ID not found.\n");
}

int main() {
    srand(time(NULL));
    int choice;
    textToSpeech("Welcome to the voice based airline reservation system.");

    do {
        textToSpeech("Main menu. Say book, view, cancel, or exit.");
        printf("\n--- Airline Reservation System ---\n");
        printf("1. Book Ticket\n2. View Booked Tickets\n3. Cancel Ticket\n4. Exit\n");
        choice = speechToMenuChoice();

        while (choice == -1) {
            textToSpeech("Invalid option. Please try again.");
            choice = speechToMenuChoice();
        }

        switch (choice) {
            case 1: bookTicket(); break;
            case 2: displayPassengers(); break;
            case 3: cancelTicket(); break;
            case 4:
                saveToFile();
                textToSpeech("Thank you. Goodbye.");
                break;
        }
    } while (choice != 4);

    return 0;
}
