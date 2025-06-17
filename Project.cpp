// Virtual ATM System
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <map>
#include <sstream>
#include <algorithm>
using namespace std;

// Utility Functions
bool isAlphaSpace(const string& str) {
    for (char ch : str) {
        if (!isalpha(ch) && ch != ' ') return false;
    }
    return true;
}

bool isDigits(const string& str) {
    return all_of(str.begin(), str.end(), ::isdigit);
}

string generateAccountNumber() {
    srand(time(0));
    return to_string(rand() % 900000 + 100000); // 6-digit number
}

void waitForEnter() {
    string input;
    cout << "\nPress Enter to continue...";
    getline(cin, input);

    while (!input.empty()) { // If user typed anything before Enter
        cout << "Invalid input. Please press only Enter to continue...";
        getline(cin, input);
    }
}


// User Class
class User {
public:
    string username;
    string password;
    string accountNumber;
    double balance;
    bool isApproved = true;
    bool rejected = false;

    User(string u = "", string p = "", double b = 0.0) : username(u), password(p), balance(b), isApproved(false) {}

    string serialize() const {
    	stringstream ss;
    	ss << username << "|" << password << "|" << (isApproved ? accountNumber : (rejected ? "REJECTED" : "PENDING")) << "|" << balance << "|" << (isApproved ? "1" : "0") << "|" << (rejected ? "1" : "0");
    	return ss.str();
	}


    static User deserialize(const string& data) {
    	stringstream ss(data);
    	string u, p, acc, balStr, appr, rej;
    	getline(ss, u, '|'); 
    	getline(ss, p, '|'); 
    	getline(ss, acc, '|'); 
    	getline(ss, balStr, '|'); 
    	getline(ss, appr, '|'); 
    	getline(ss, rej); // now read rejected flag

    	User user(u, p, stod(balStr));
    	user.accountNumber = acc;
    	user.isApproved = (appr == "1");
    	user.rejected = (rej == "1");
    	return user;
	}

};

// File Handling
vector<User> loadUsers(const string& filename) {
    vector<User> users;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (!line.empty())
            users.push_back(User::deserialize(line));
    }
    return users;
}

void saveUsers(const vector<User>& users, const string& filename) {
    ofstream file(filename);
    for (const auto& user : users) {
        file << user.serialize() << "\n";
    }
}

User* findUser(vector<User>& users, const string& username) {
    for (auto& user : users) {
        if (user.username == username)
            return &user;
    }
    return nullptr;
}

User* findUserByAccountNumber(vector<User>& users, const string& accNum) {
    for (auto& user : users) {
        if (user.accountNumber == accNum)
            return &user;
    }
    return nullptr;
}

void signup(vector<User>& users) {
    string username, password, depositStr;
    double deposit = 0.0;

    while (true) {
        cout << "Enter a unique username (alphabets only) or 0 to cancel: ";
        getline(cin, username);
        if (username == "0") { cout<<endl; return;}
        if (!isAlphaSpace(username)) { cout << "Invalid username.\n"; continue; }
        if (findUser(users, username)) { cout << "Username already exists.\n"; continue; }
        break;
    }

    while (true) {
        cout << "Enter a pin (4 digits) or 0 to cancel: ";
        getline(cin, password);
        if (password == "0") return;
        if (!isDigits(password) || password.length() != 4) {
            cout << "Invalid password.\n"; continue;
        }
        break;
    }

    while (true) {
        cout << "Enter initial deposit or 0 to cancel: ";
        getline(cin, depositStr);
        if (depositStr == "0") return;
        try {
            deposit = stod(depositStr);
            if (deposit < 0) throw invalid_argument("Negative");
            break;
        } catch (...) {
            cout << "Invalid amount.\n";
        }
    }

    users.emplace_back(username, password, deposit);
    saveUsers(users, "users.txt");
    cout << "Your account will be reviewed by the admin shortly.\n";
    waitForEnter();
    cout<<endl;
}

bool adminLogin() {
    string user, pass1, pass2;
    while (true) {
        cout << "Enter admin username or 0 to exit: "; getline(cin, user);
        if (user == "0") return false;
        if (user != "admin") { cout << "Invalid username.\n"; continue; }
        cout << "Enter first password: "; getline(cin, pass1);
        if (pass1 == "0") return false;
        cout << "Enter second password: "; getline(cin, pass2);
        if (pass2 == "0") return false;
        if (pass1 == "1234" && pass2 == "5678") return true;
        else cout << "Incorrect passwords.\n";
    }
}

void viewAccounts(const vector<User>& users) {
    cout << "All Accounts:\n";
    for (const auto& user : users) {
        cout << user.username << " - " << (user.isApproved ? user.accountNumber : (user.rejected?"rejected":"pending")) << " - Balance: " << user.balance << "\n";
    }
    waitForEnter();
}

void viewPendingRequests(vector<User>& users) {
    int pendingCount = 0;

    for (const auto& user : users) {
        if (!user.isApproved && !user.rejected) {
            pendingCount++;
        }
    }

    cout << "\nYou have " << pendingCount << " pending request(s).\n";

    for (auto& user : users) {
        if (!user.isApproved && !user.rejected) {
            cout << "\nPending request: " << user.username << "\n";

            while (true) {
                cout << "1. Accept\n2. Reject\n3. Skip\n0. Exit\nEnter choice: ";
                int c;
                cin >> c;
                cin.ignore(); // flush newline

                if (c == 1) {
                    user.accountNumber = generateAccountNumber();
                    user.isApproved = true;
                    cout << "Approved. Account number: " << user.accountNumber << "\n";
                    break;
                } else if (c == 2) {
                    user.rejected = true;
                    cout << user.username << " has been rejected.\n";
                    break;
                } else if (c == 3) {
                    break;
                } else if (c == 0) {
                    saveUsers(users, "users.txt");
                    return;
                } else {
                    cout << "Invalid input. Try again.\n";
                }
            }
        }
    }

    saveUsers(users, "users.txt");
    cout << "No more pending requests.\n";
    waitForEnter();
}


void transactionHistory(const string& username) {
    ifstream file("trans_" + username + ".txt");
    if (!file) {
        cout << "No transaction history found for " << username << ".\n";
        return;
    }
    string line;
    cout << "Transaction history for " << username << ":\n";
    while (getline(file, line)) {
        cout << line << '\n';
    }
}

void deleteAccount(vector<User>& users) {
    string username;
    cout << "Enter username to delete: ";
    getline(cin, username);

    User* user = findUser(users, username);
    if (!user) {
        cout << "User not found.\n";
        waitForEnter();
        return;
    }

    string confirm;
    cout << "Are you sure you want to delete " << username << "'s account? (y/n): ";
    getline(cin, confirm);

    if (confirm == "y" || confirm == "Y") {
        auto it = remove_if(users.begin(), users.end(), [&](User& u) { return u.username == username; });
        if (it != users.end()) users.erase(it, users.end());
        saveUsers(users, "users.txt");
        cout << "Account deleted.\n";
    } else if (confirm == "n" || confirm == "N") {
        cout << "Deletion cancelled.\n";
    } else {
    	cout << "Invalid choice.\n";
	}
    waitForEnter();
}

void adminPanel(vector<User>& users) {
    while (true) {
        cout << "\nAdmin Menu:\n1. View All Accounts\n2. View Pending Requests\n3. View Transaction History\n4. Delete Account\n5. Exit\nChoice: ";
        string choice; getline(cin, choice);
        if (choice == "1") viewAccounts(users);
        else if (choice == "2") viewPendingRequests(users);
        else if (choice == "3") {
		    string user;
		    cout << "Enter username or 0 to exit: ";
		    getline(cin, user);
		
		    if (user == "0") return;
		
		    // Check if username exists
		    bool found = false;
		    for (const auto& u : users) {
		        if (u.username == user && u.isApproved) {
		            found = true;
		            break;
		        }
		    }
		
		    if (!found) {
		        cout << "Username not found or not approved.\n";
		    } else {
		        transactionHistory(user);
		    }
		
		    waitForEnter();
		}

        else if (choice == "4") deleteAccount(users);
        else if (choice == "5") { cout<<endl; break; }
        else cout << "Invalid input.\n";
    }
}

void login(vector<User>& users) {
    string username, password;
    cout << "Enter username: "; getline(cin, username);
    cout << "Enter password: "; getline(cin, password);
    User* user = findUser(users, username);
    if (!user || user->password != password) {
        cout << "Invalid login.\n";
        waitForEnter();
        cout<<endl;
        return;
    }
    if (user->rejected) {
    	cout << "Your account has been rejected by the admin.\n";
    	cout << "Choose an option:\n1. Re-apply with a new username\n2. Exit\nChoice: ";
    	string opt; getline(cin, opt);
    	if (opt == "1") {
        	signup(users);  // Allow user to reapply
    	} else {
    	    cout << "Goodbye.\n";
    	}
    	return;
	}

    if (!user->isApproved) {
        cout << "Your account is being reviewed. Please wait.\n";
        waitForEnter();
        cout<<endl;
        return;
    }
    cout << "Login successful. Welcome " << user->username << ". Account #: " << user->accountNumber << "\n";
    while (true) {
        cout << "\nCustomer Menu:\n1. Check Balance\n2. Deposit\n3. Withdraw\n4. Transfer\n5. Change Password\n6. Logout\nChoice: ";
        string choice; getline(cin, choice);
        if (choice == "1") cout << "Balance: " << user->balance << "\n";
        else if (choice == "2") {
            string amt; cout << "Enter amount: "; getline(cin, amt);
            if (isDigits(amt)) { user->balance += stod(amt); cout << "New Balance: " << user->balance << "\n"; }
            else cout << "Invalid amount.\n";
        }
        else if (choice == "3") {
    		string amt;
    		cout << "Enter amount: ";
    		getline(cin, amt);
    		try {
        		double amount = stod(amt);
        		if (amount <= 0) {
            		cout << "Amount must be greater than zero.\n";
        		} else if (amount > user->balance) {
            		cout << "Insufficient funds.\n";
        		} else {
            		user->balance -= amount;
            		cout << "Remaining Balance: " << user->balance << "\n";
        		}
    		} catch (...) {
        		cout << "Invalid input. Please enter a valid number.\n";
    		}
		}

        else if (choice == "4") {
    		string acc;
    		cout << "Enter account number to transfer: ";
    		getline(cin, acc);
    		User* target = findUserByAccountNumber(users, acc);
    		if (!target) {
        		cout << "Invalid account number.\n";
        		continue;
    		}
    		string amt;
    		cout << "Enter amount: ";
    		getline(cin, amt);
    		try {
        		double amount = stod(amt);
        		if (amount <= 0) {
            		cout << "Amount must be greater than zero.\n";
        		} else if (amount > user->balance) {
            		cout << "Insufficient funds.\n";
        		} else {
            		user->balance -= amount;
            		target->balance += amount;

            		ofstream file("trans_" + user->username + ".txt", ios::app);
            		file << "Transferred " << amount << " to " << target->username << "\n";
            		file = ofstream("trans_" + target->username + ".txt", ios::app);
            		file << "Received " << amount << " from " << user->username << "\n";

            		cout << "Transferred successfully.\n";
        		}
    		} catch (...) {
        		cout << "Invalid input. Please enter a valid number.\n";
    		}
		}

        else if (choice == "5") {
            string cur, npass, cpass;
            cout << "Current password: "; getline(cin, cur);
            if (cur != user->password) { cout << "Incorrect password.\n"; continue; }
            cout << "New password: "; getline(cin, npass);
            cout << "Confirm new password: "; getline(cin, cpass);
            if (npass != cpass || !isDigits(npass) || npass.length() < 4) {
                cout << "Password change failed.\n"; continue;
            }
            user->password = npass;
            cout << "Password changed.\n";
        }
        else if (choice == "6") { cout<<endl; break; }
        else cout << "Invalid input.\n";
        saveUsers(users, "users.txt");
    }
}

int main() {
    vector<User> users = loadUsers("users.txt");
    while (true) {
        cout << "=== Welcome to Cash Nova ===\nMain Menu:\n1. Login\n2. Signup\n3. Admin Panel\n4. Exit\nChoice: ";
        string choice; getline(cin, choice);
        if (choice == "1") login(users);
        else if (choice == "2") signup(users);
        else if (choice == "3") {
            if (adminLogin()) adminPanel(users);
        }
        else if (choice == "4") break;
        else cout << "Invalid input.\n";
    }
    return 0;
}