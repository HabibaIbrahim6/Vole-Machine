#include"VoleMachine.h"

int main() {
    CPU cpu;
    string filename;
    cout << "Enter file name: ";
    cin >> filename;
    cpu.loadInstructions(filename);

    int choice;
    do {
        cout << "\nMenu:\n";
        cout << "1. Execute all at once\n";
        cout << "2. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            cpu.executeAllAtOnce(cout);
            break;
        case 2:
            cout << "Exiting program...\n";
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
        }
    } while (choice != 2);

    return 0;
}
