#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

int no_of_players;
int total_points;
int no_of_levels;
vector<int> player_points;
queue<pair<int, int>> lobby;

pthread_mutex_t lobby_lock;
vector<pthread_cond_t> lobby_conds;
pthread_mutex_t timeline_lock;

struct check_point {
    int player;
    int level;
    int level_points;
};
vector<check_point> timeline;

void transfer_points(int player1, int player2, int points) {
    assert(player_points[player1] >= points);

    player_points[player1] -= points;
    player_points[player2] += points;

    printf("Player %d to Player %d: %d points.\n", player1, player2, points);
}

void check_lobby(int player) {
    while (!lobby.empty() && player_points[player] > 0) {
        if (lobby.front().second < 0) {
            transfer_points(lobby.front().first, player, abs(lobby.front().second));
            lobby.pop();
        }
        else {
            int s = min(player_points[player], lobby.front().second);
            transfer_points(player, lobby.front().first, s);
            if (s == lobby.front().second) {
                pthread_cond_signal(&lobby_conds[lobby.front().first]);
                lobby.pop();
            }
            else {
                lobby.front().second -= s;
            }
        }
    }
}

void* play(void* arg) {
    int player = *((int *) arg);
    printf("Player %d entered the arena.\n", player);

    for (int level = 1; level <= no_of_levels; ++level) {
        int level_points = 1 + rand() % total_points / 2;
        // int level_points = total_points;

        pthread_mutex_lock(&lobby_lock);
        printf("Player %d need %d pointes to enter level %d. He has %d points.\n", player, level_points, level, player_points[player]);
        if (player_points[player] < level_points) {
            check_lobby(player);
            if (player_points[player] < level_points) {
                lobby.push({player, level_points - player_points[player]});
                printf("Player %d is waiting in the lobby.\n", player);
                pthread_cond_wait(&lobby_conds[player], &lobby_lock);
            }
        }
        assert(player_points[player] >= level_points);
        pthread_mutex_unlock(&lobby_lock);

        printf("Player %d started playing level %d.\n", player, level);
        sleep(1);
        printf("Player %d successfully completed level %d.\n", player, level);

        pthread_mutex_lock(&timeline_lock);
        timeline.push_back({player, level, level_points});
        pthread_mutex_unlock(&timeline_lock);
    }

    pthread_mutex_lock(&lobby_lock);
    check_lobby(player);
    if (player_points[player] > 0) {
        lobby.push({player, -player_points[player]});
    }
    pthread_mutex_unlock(&lobby_lock);

    return NULL;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Invalid usage: ./main no_of_players total_points no_of_levels\n");
        exit(1);
    }
    no_of_players = atoi(argv[1]);
    total_points = atoi(argv[2]);
    no_of_levels = atoi(argv[3]);

    pthread_mutex_init(&lobby_lock, NULL);
    lobby_conds.resize(no_of_players);
    for (int i = 0; i < no_of_players; ++i) {
        pthread_cond_init(&lobby_conds[i], NULL);
    }

    printf("Number of players: %d\n", no_of_players);

    player_points.resize(no_of_players);
    for (int i = 0; i < no_of_players; ++i) {
        player_points[i] = (total_points / no_of_players) + (i < (total_points % no_of_players));
    }

    printf("Initial point distribution: ");
    for (int i = 0; i < no_of_players; ++i) {
        printf("(%d, %d)%c", i, player_points[i], " \n"[i == no_of_players-1]);
    }
    printf("Total points = %d\n", accumulate(player_points.begin(), player_points.end(), 0));
    assert(accumulate(player_points.begin(), player_points.end(), 0) == total_points);

    int* player_ids = new int[no_of_players];
    pthread_t* player_threads = new pthread_t[no_of_players];
    for (int i = 0; i < no_of_players; ++i) {
        player_ids[i] = i;
        pthread_create(&player_threads[i], NULL, play, (void*) &player_ids[i]);
    }
    for (int i = 0; i < no_of_players; ++i) {
        pthread_join(player_threads[i], NULL);
    }

    printf("--- Timeline ---\n");
    for (auto [player, level, level_points] : timeline) {
        printf("Player %d cleared level %d with %d points.\n", player, level, level_points);
    }

    printf("Final point distribution: ");
    for (int i = 0; i < no_of_players; ++i) {
        printf("(%d, %d)%c", i, player_points[i], " \n"[i == no_of_players-1]);
    }
    printf("Total points = %d\n", accumulate(player_points.begin(), player_points.end(), 0));
    assert(accumulate(player_points.begin(), player_points.end(), 0) == total_points);
    printf("Arena conquered!");

    delete[] player_ids;
    delete[] player_threads;

    return 0;
}