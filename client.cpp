#include <iostream>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "single_include/nlohmann/json.hpp"
#include "helpers.h"
#include "requests.h"
#include <string>
#include <unordered_set>

using json = nlohmann::json;
using namespace std;

unordered_set<string> availableFirstCommands() {
    unordered_set<string> set;

    set.insert("register");
    set.insert("login");
    set.insert("enter_library");
    set.insert("get_books");
    set.insert("get_book");
    set.insert("add_book");
    set.insert("delete_book");
    set.insert("logout");
    set.insert("exit");

    return set;
}

void reg(int sockfd) {
    string username, password;
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, password);
    if(username.find(' ') != std::string::npos || 
        password.find(' ') != std::string::npos) {
        cout << "Invalid username and/or password!\n";
        return;
    }

    json j;
    j["username"] = username;
    j["password"] = password;

    char **json_data = (char**)calloc(1, sizeof(char *));
    json_data[0] = (char*)calloc(j.dump().length()+1, sizeof(char));
    strcpy(json_data[0], j.dump().c_str());

    char *message = compute_post_request("34.241.4.235", 
        "/api/v1/tema/auth/register", "application/json", json_data, 1, NULL,
         0, NULL);
    send_to_server(sockfd, message);

    free(json_data[0]);
    free(json_data);
    free(message);

    char *response = receive_from_server(sockfd);

    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "201", 3) == 0) {
        cout << "Account registered successfully!\n";
    } else if(strncmp(begin_with_code + 1, "400", 3) == 0) {
        char *json_response = basic_extract_json_response(response);
        json j2 = json::parse(json_response);
        if(j2.contains("error")) {
            cout << j2.at("error") << endl;
        } else {
            cout << "Bad request!\n";
        }
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }
}

string login(int sockfd) {
    string username, password;
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, password);
    if(username.find(' ') != std::string::npos ||
        password.find(' ') != std::string::npos) {
        cout << "Invalid username and/or password!\n";
        return "";
    }

    json j;
    j["username"] = username;
    j["password"] = password;

    char **json_data = (char**)calloc(1, sizeof(char *));
    json_data[0] = (char*)calloc(j.dump().length()+1, sizeof(char));
    strcpy(json_data[0], j.dump().c_str());

    char *message = compute_post_request("34.241.4.235", 
    "/api/v1/tema/auth/login", "application/json", json_data, 1, NULL, 0, NULL);
    
    send_to_server(sockfd, message);

    free(json_data[0]);
    free(json_data);
    free(message);

    char *response = receive_from_server(sockfd);

    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "200", 3) == 0) {
        cout << "Logged in successfully!\n";
        char* cookie_start = strstr(response, "connect.sid");
        char* cookie_finish = strchr(cookie_start, ';');
        string s;
        s.assign(cookie_start, cookie_finish - cookie_start);
        return s;
    } else if(strncmp(begin_with_code + 1, "400", 3) == 0) {
        char *json_response = basic_extract_json_response(response);
        json j2 = json::parse(json_response);
        if(j2.contains("error")) {
            cout << j2.at("error") << endl;
        } else {
            cout << "Bad request!\n";
        }
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }

    return "";
}

json access(int sockfd, string sessionCookie) {
    char *message, *response;

    char **cookies = (char**)calloc(1, sizeof(char *));
    cookies[0] = (char*)calloc(sessionCookie.length() + 1, sizeof(char));
    strcpy(cookies[0], sessionCookie.c_str());

    message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access",
        NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);
    free(message);

    response = receive_from_server(sockfd);

    json j;

    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "200", 3) == 0) {
        cout << "Access granted!\n";
        char *json_response = basic_extract_json_response(response);
        j = json::parse(json_response);
        return j;
    } else if(strncmp(begin_with_code + 1, "401", 3) == 0) {
        char *json_response = basic_extract_json_response(response);
        json j2 = json::parse(json_response);
        cout << j2.at("error") << endl;
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }
    
    return j;
}

void get_books(int sockfd, json token_json) {
    char *message, *response;

    if(token_json.contains("token") == false) {
        cout << "No acces to the library!\n";
        return;
    }

    string token_value = token_json.at("token"); 
    char* auth = (char*)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books",
        NULL, NULL, 0, auth);

    send_to_server(sockfd, message);

    free(auth);
    free(message);

    response = receive_from_server(sockfd);

    if(strstr(response, "[]") != NULL) {
        cout << "Lista este goala!\n";
        return;
    }
    json resp_json = json::parse(strchr(response, '['));
    cout << resp_json.dump(4) << endl;
}

void get_book(int sockfd, json token_json) {
    char *message, *response;

    if(token_json.contains("token") == false) {
        cout << "No acces to the library!\n";
        return;
    }

    string id;
    cout << "id=";
    getline(cin, id);

    if(id.length() == 0) {
        cout << "Invalid ID!\n";
        return;
    }
    for(size_t i = 0; i < id.length(); i++) {
        if(id.at(i) < '0' || id.at(i) > '9') {
            cout << "Invalid ID!\n";
            return;
        }
    }

    string url = "/api/v1/tema/library/books/";
    url += id;

    string token_value = token_json.at("token"); 
    char* auth = (char*)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    message = compute_get_request("34.241.4.235", url.c_str(), NULL, NULL,
        0, auth);
    send_to_server(sockfd, message);

    free(auth);
    free(message);

    response = receive_from_server(sockfd);
    
    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "200", 3) == 0) {
        json resp_json = json::parse(strchr(response, '['));
        cout << resp_json.dump(4) << endl;
    } else if(strncmp(begin_with_code + 1, "404", 3) == 0) {
        cout << "No book with this ID!\n";
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }
} 

void add_book(int sockfd, json token_json) {
    if(token_json.contains("token") == false) {
        cout << "No acces to the library!!\n";
        return;
    }

    string title, author, genre, publisher, page_count;
    cout << "title=";
    getline(cin, title);
    cout << "author=";
    getline(cin, author);
    cout << "genre=";
    getline(cin, genre);
    cout << "publisher=";
    getline(cin, publisher);
    cout << "page_count=";
    getline(cin, page_count);

    if(title.length() * author.length() * genre.length() * publisher.length() *
         page_count.length() == 0) {
        cout << "Invalid book informations!\n";
        return;
    }
    for(size_t i = 0; i < page_count.length(); i++) {
        if(page_count.at(i) < '0' || page_count.at(i) > '9') {
            cout << "Invalid number of pages!\n";
            return;
        }
    }

    json j;
    j["title"] = title;
    j["author"] = author;
    j["genre"] = genre;
    j["publisher"] = publisher;
    j["page_count"] = page_count;

    string token_value = token_json.at("token"); 
    char* auth = (char*)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    char **json_data = (char**)calloc(1, sizeof(char *));
    json_data[0] = (char*)calloc(j.dump().length() + 1, sizeof(char));
    strcpy(json_data[0], j.dump().c_str());

    char *message = compute_post_request("34.241.4.235",
        "/api/v1/tema/library/books", "application/json", json_data, 1, NULL,
        0, auth);
    send_to_server(sockfd, message);

    free(auth);
    free(json_data[0]);
    free(json_data);
    free(message);

    char *response = receive_from_server(sockfd);

    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "200", 3) == 0) {
        cout << "Book succesfully added!\n";
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }
}

void delete_book(int sockfd, json token_json) {
    char *message, *response;

    if(token_json.contains("token") == false) {
        cout << "No acces to the library!!\n";
        return;
    }

    string id;
    cout << "id=";
    getline(cin, id);

    if(id.length() == 0) {
        cout << "ID invalid!\n";
        return;
    }
    for(size_t i = 0; i < id.length(); i++) {
        if(id.at(i) < '0' || id.at(i) > '9') {
            cout << "Invalid ID!\n";
            return;
        }
    }

    string url = "/api/v1/tema/library/books/";
    url += id;

    string token_value = token_json.at("token"); 
    char* auth = (char*)calloc(token_value.length() + 1, sizeof(char));
    strcpy(auth, token_value.c_str());

    message = compute_delete_request("34.241.4.235", url.c_str(), NULL, NULL,
        0, auth);
    send_to_server(sockfd, message);

    free(auth);
    free(message);

    response = receive_from_server(sockfd);
    
    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "200", 3) == 0) {
        cout << "Book succesfully deleted!\n";
    } else if(strncmp(begin_with_code + 1, "404", 3) == 0) {
        cout << "No book with this ID!\n";
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }
}

int logout(int sockfd, string &sessionCookie) {
    char *message, *response;

    char **cookies = (char**)calloc(1, sizeof(char *));
    cookies[0] = (char*)calloc(sessionCookie.length() + 1, sizeof(char));
    strcpy(cookies[0], sessionCookie.c_str());

    message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout",
        NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    free(cookies[0]);
    free(cookies);
    free(message);

    response = receive_from_server(sockfd);

    char *begin_with_code = strchr(response, ' ');
    if(strncmp(begin_with_code + 1, "200", 3) == 0) {
        cout << "You've been logged out!\n";
        sessionCookie.clear();
        return 1;
    } else if(strncmp(begin_with_code + 1, "400", 3) == 0) {
        cout << "You are not logged in!\n";
    } else if(strncmp(begin_with_code + 1, "429", 3) == 0) {
        cout << "Too many requests! Try again later!\n";
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd;

    string firstCommand, sessionCookie;
    unordered_set<string> available = availableFirstCommands();
    json token_json;
    
    while(1) {
        if(sockfd <= 0) {
            cout << "Connection failed\n";
            return 0;
        }
        getline(cin, firstCommand);
        sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
        if (available.count(firstCommand) == 0)  {
            cout << "Invalid command\n";
            continue;
        }

        if(firstCommand.compare("exit") == 0) {
            close(sockfd);
            break;
        }

        if(firstCommand.compare("register") == 0) {
            if(!sessionCookie.empty()) {
                cout << "User already connected!\n";
                continue;
            }
            reg(sockfd);
        }

        if(firstCommand.compare("login") == 0) {
            if(!sessionCookie.empty()) {
                cout << "User already connected!\n";
                continue;
            }
            sessionCookie = login(sockfd);
        }

        if(firstCommand.compare("enter_library") == 0) {
            token_json = access(sockfd, sessionCookie);
        }

        if(firstCommand.compare("get_books") == 0) {
            get_books(sockfd, token_json);
        }

        if(firstCommand.compare("add_book") == 0) {
            add_book(sockfd, token_json);
        }

        if(firstCommand.compare("get_book") == 0) {
            get_book(sockfd, token_json);
        }

        if(firstCommand.compare("delete_book") == 0) {
            delete_book(sockfd, token_json);
        }

        if(firstCommand.compare("logout") == 0) {
            if(logout(sockfd, sessionCookie) == 1) {
                token_json.clear();
            }
        }
        close(sockfd);
    }

    return 0;
}