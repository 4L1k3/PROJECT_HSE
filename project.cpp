#include <iostream>
#include <vector>
#include <cstdlib>
#include <map>
#include <string>

const int P = 20, P_STS = 68, P_SA = 80, P_S = 70, D_STS = 1, D_SA = 10, V_LMAX = 5, V_MAX = 8;
const std::vector<std::string> signlas = {"G", "R"};

std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> nexts;
std::map<std::pair<int, int>, int> rd_len;
std::map<std::pair<int, int>, std::string> start_signal;
int TIME_GREEN, TIME_RED;

class CAR {
public:
	int v;
	int n;
	int g;
	int b;
	int id;
	std::string LS;
	std::pair<int, int> road_id;
	int road_len;

	CAR(std::pair<int, int> _road_id, int _road_len, int _n, int _v, int _id) {
		road_id = _road_id;
		road_len = _road_len;
		n = _n;
		v = _v;
		id = _id;
		b = 0;
		LS = start_signal[road_id];
	}
};

std::vector<CAR> cars;
std::vector<CAR> new_cars;

int EPS() {
	return std::rand() % 100 + 1;
}

std::pair<int, int> get_next_road(std::pair<int, int> road_id) {
	return nexts[road_id][0]; // МОЖНО РАНДОМИЗИРОВАТЬ ТАК [std::rand() % nexts[road_id].size()];
}

int get_first_free(std::pair<int, int> rd_id) {
	int mn = rd_len[rd_id];
	for (size_t i = 0; i < cars.size(); ++i) {
		if (cars[i].road_id == rd_id) {
			if (cars[i].n < mn) {
				mn = cars[i].n;
			}
		}
	}
	return mn - 1;
}

int get_last_free_for_car(std::pair<int, int> rd_id, int car_n) {
	int mn = rd_len[rd_id];
	for (size_t i = 0; i < cars.size(); ++i) {
		if (cars[i].road_id == rd_id) {
			if (cars[i].n > car_n && cars[i].n < mn) {
				mn = cars[i].n;
			}
		}
	}
	return mn - 1;
}

int get_first_car_on_road(std::pair<int, int> rd_id) {
	int mn = rd_len[rd_id];
	int ans = -1;
	for (size_t i = 0; i < cars.size(); ++i) {
		if (cars[i].road_id == rd_id) {
			if (cars[i].n < mn) {
				mn = cars[i].n;
				ans = i;
			}
		}
	}
	return ans;
}

std::pair<CAR, int> get_next_car(CAR c) {
	int last = c.road_len;
	int ans = -1;
	for (size_t i = 0; i < cars.size(); ++i) {
		if (cars[i].road_id == c.road_id) {
			if (cars[i].n > c.n && cars[i].n < last) {
				last = cars[i].n;
				ans = i;
			}
		}
	}
	int dist = last - c.n - 1;	
	if (last == c.road_len) {
		std::pair<int, int> road_id = get_next_road(c.road_id);
		ans = get_first_car_on_road(road_id);
		while (ans == -1) {
			dist += rd_len[road_id];
			road_id = get_next_road(road_id);
			ans = get_first_car_on_road(road_id);
		}
		dist += cars[ans].n ;
	}
	return {cars[ans], dist};
}

void update_car(int id, int v, int n, int b, int time) {
	new_cars[id].v = v;
	new_cars[id].n = n;
	new_cars[id].b = b;
	time %= TIME_RED + TIME_GREEN;
	if (start_signal[new_cars[id].road_id] == "R") {
		if (time < TIME_RED) {
			new_cars[id].LS = "R";
		} else {
			new_cars[id].LS = "G";
		}
	}
	if (start_signal[new_cars[id].road_id] == "G") {
		if (time < TIME_GREEN) {
			new_cars[id].LS = "G";
		} else {
			new_cars[id].LS = "R";
		}
	}
}

void update_road(CAR c) {
	new_cars[c.id].road_id = get_next_road(cars[c.id].road_id);
	new_cars[c.id].road_len = rd_len[new_cars[c.id].road_id];
}

void update(CAR c, int time) {
	int new_v = c.v;
	int old_n = c.n;
	int old_v = c.v;
	int old_g = c.g;
	int old_b = c.b;
	// УСКОРЕНИЕ
	if (EPS() < P_STS && old_v == 0 && old_g <= D_STS) {
		new_v = 0;
	}
	new_v = std::min(new_v, std::min(old_v + 1, V_LMAX));
	// ТОРМОЖЕНИЕ
	CAR next_c = get_next_car(c).first;
	int new_b = 0;
	if (EPS() < P_SA && old_v > 0 && next_c.v > 0 
		&& old_g <= D_SA 
		&& (next_c.b == 1 or next_c.v < old_v)) {
		new_v = next_c.v;
		new_b = 1;
	}
	if (new_v > old_g) {
		new_v = old_g;
		new_b = 1;
	}
	// СЛУЧАЙНОЕ ЗАМЕДЛЕНИЕ
	if (EPS() < P && old_v > 1) {
		new_v = std::max(new_v - 1, 0);
	}
	// ПРЕВЫШЕНИЕ СКОРОСТИ
	if (EPS() < P_S && old_v == V_LMAX
		&& old_v + 1 < old_g && old_v + 1 < V_MAX) {
		new_v = old_v + 1;
	}
	// СВЕТОФОР
	int g_ls = c.road_len - old_n;
	if (g_ls <= V_MAX + 1) {
		std::string LS = c.LS;
		if (LS == "R" && old_g > g_ls && g_ls > 0 && new_v >= 0) {
			new_v = 1;
		}
		if (LS == "R" && old_g > g_ls && g_ls == 0 && new_v >= 1) {
			new_v = 0;
			new_b = 1;
		}
	}
	// ДВИЖЕНИЕ И ПОВОРОТЫ
	int new_n = old_n + new_v;
	if (new_n >= c.road_len) {
		int u = new_n + 1 - c.road_len;
		int nfb = get_first_free(get_next_road(c.road_id));
		if (nfb >= u - 1) {
			new_n = u - 1;
		} else {
			if (nfb >= 0) {
				new_n = nfb;
				new_v = c.road_len + nfb - old_n;
			} else {
				int nfa = get_last_free_for_car(c.road_id, old_n);
				new_n = nfa;
				new_v = nfa - new_n;
			}
		}
		update_road(c);
	}
	update_car(c.id, new_v, new_n, new_b, time);
}

std::map<int, bool> used;
int generate_n(int len) {
	int ans = std::rand() % len;
	while (used[ans]) {
		ans = std::rand() % len;
	}
	used[ans] = true;
	return ans;
}

void add_to_road(std::pair<int, int> rd_id, int len, int count) {
	for (size_t i = 0; i < count; ++i) {
		cars.push_back(CAR(rd_id, len, generate_n(100), std::rand() % (V_LMAX + 1), cars.size()));	
	}
}

void update_g(int time) {
	for (size_t i = 0; i < cars.size(); ++i) {
		cars[i].g = get_next_car(cars[i]).second;
	}
}

generate_start_signals(int n) {
	int k = 0;
	for (int i = 0; i < n - 1; ++i) {
		for (int j = i + 1; j < n; ++j) {
			start_signal[{i, j}] = signlas[k % 2];
			start_signal[{j, i}] = signlas[k % 2];
			k++;
		}
	}
}

void START_OF_SIMULATION(int T) {
	for (size_t t = 0; t < T; ++t) {
		new_cars = cars;
		for (size_t i = 0; i < cars.size(); ++i) {
			update(cars[i], t + 1);
		}
		cars = new_cars;
		update_g(t);
	}
}

int main() {
	freopen("input.txt", "r", stdin);
	freopen("output.txt", "w", stdout);
	std::cin >> TIME_RED >> TIME_GREEN;
	int N = 3;
	generate_start_signals(N);
	std::vector<std::pair<int, int>> roads_ids = {{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}};
	nexts[{0, 1}] = {{1, 2}};
	nexts[{1, 0}] = {{0, 2}};
	nexts[{0, 2}] = {{2, 1}};
	nexts[{2, 0}] = {{0, 1}};
	nexts[{1, 2}] = {{2, 0}};
	nexts[{2, 1}] = {{1, 0}};

	rd_len[{0, 1}] = 100;
	rd_len[{1, 0}] = 100;
	rd_len[{0, 2}] = 150;
	rd_len[{2, 0}] = 150;
	rd_len[{1, 2}] = 200;
	rd_len[{2, 1}] = 200;
	
	add_to_road({0, 1}, 100, 3);
	add_to_road({1, 0}, 100, 1);
	add_to_road({2, 0}, 150, 2);
	add_to_road({0, 2}, 150, 1);
	add_to_road({1, 2}, 200, 1);
	add_to_road({2, 1}, 200, 1);
	
	update_g(0);
	for (size_t i = 0; i < roads_ids.size(); ++i) {
		
		std::cout << "ROAD FROM " << roads_ids[i].first << " TO " << roads_ids[i].second << "\n";
		std::cout << "START SIGNAL = " << start_signal[roads_ids[i]] << "\n";
		for (size_t j = 0; j < cars.size(); ++j) {
			if (cars[j].road_id == roads_ids[i]) {
				CAR c = cars[j];
				std::cout << "CAR_ID = " << c.id << "\n";
				std::cout << " position on the road = " << c.n << "\n";
				std::cout << " distance to the next car = " << c.g << "\n";
				std::cout << " speed = " << c.v << "\n";
				std::cout << " LS = " << c.LS << "\n";
			}
		}
		std::cout << "\n";
	}
	int T = 150000;
	START_OF_SIMULATION(T);
	std::cout << "#############\n";
	std::cout << "AFTER " << T << " SEC\n\n\n";
	for (size_t i = 0; i < roads_ids.size(); ++i) {
		
		std::cout << "ROAD FROM " << roads_ids[i].first << " TO " << roads_ids[i].second << "\n";
		std::cout << "START SIGNAL = " << start_signal[roads_ids[i]] << "\n";
		for (size_t j = 0; j < cars.size(); ++j) {
			if (cars[j].road_id == roads_ids[i]) {
				CAR c = cars[j];
				std::cout << "CAR_ID = " << c.id << "\n";
				std::cout << " position on the road = " << c.n << "\n";
				std::cout << " distance to the next car = " << c.g << "\n";
				std::cout << " speed = " << c.v << "\n";
				std::cout << " LS = " << c.LS << "\n";
			}
		}
		std::cout << "\n";
	}
	return 0;
}
