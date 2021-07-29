/* Copyright 2021 Aristocratos (jakob@qvantnet.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

indent = tab
tab-size = 4
*/

#include <array>
#include <algorithm>
#include <cmath>
#include <ranges>

#include <btop_draw.hpp>
#include <btop_config.hpp>
#include <btop_theme.hpp>
#include <btop_shared.hpp>
#include <btop_tools.hpp>
#include <btop_input.hpp>


using 	std::round, std::views::iota, std::string_literals::operator""s, std::clamp, std::array, std::floor, std::max, std::min,
		std::to_string;

using namespace Tools;
namespace rng = std::ranges;

namespace Symbols {
	const string h_line				= "─";
	const string v_line				= "│";
	const string dotted_v_line		= "╎";
	const string left_up			= "┌";
	const string right_up			= "┐";
	const string left_down			= "└";
	const string right_down			= "┘";
	const string round_left_up		= "╭";
	const string round_right_up		= "╮";
	const string round_left_down	= "╰";
	const string round_right_down	= "╯";
	const string title_left_down	= "┘";
	const string title_right_down	= "└";
	const string title_left			= "┐";
	const string title_right		= "┌";
	const string div_right			= "┤";
	const string div_left			= "├";
	const string div_up				= "┬";
	const string div_down			= "┴";


	const string up = "↑";
	const string down = "↓";
	const string left = "←";
	const string right = "→";
	const string enter = "↲";

	const string meter = "■";

	const array<string, 10> superscript = { "⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹" };

	const unordered_flat_map<string, vector<string>> graph_symbols = {
		{ "braille_up", {
			" ", "⢀", "⢠", "⢰", "⢸",
			"⡀", "⣀", "⣠", "⣰", "⣸",
			"⡄", "⣄", "⣤", "⣴", "⣼",
			"⡆", "⣆", "⣦", "⣶", "⣾",
			"⡇", "⣇", "⣧", "⣷", "⣿"
		}},
		{"braille_down", {
			" ", "⠈", "⠘", "⠸", "⢸",
			"⠁", "⠉", "⠙", "⠹", "⢹",
			"⠃", "⠋", "⠛", "⠻", "⢻",
			"⠇", "⠏", "⠟", "⠿", "⢿",
			"⡇", "⡏", "⡟", "⡿", "⣿"
		}},
		{"block_up", {
			" ", "▗", "▗", "▐", "▐",
			"▖", "▄", "▄", "▟", "▟",
			"▖", "▄", "▄", "▟", "▟",
			"▌", "▙", "▙", "█", "█",
			"▌", "▙", "▙", "█", "█"
		}},
		{"block_down", {
			" ", "▝", "▝", "▐", "▐",
			"▘", "▀", "▀", "▜", "▜",
			"▘", "▀", "▀", "▜", "▜",
			"▌", "▛", "▛", "█", "█",
			"▌", "▛", "▛", "█", "█"
		}},
		{"tty_up", {
			" ", "░", "░", "▒", "▒",
			"░", "░", "▒", "▒", "█",
			"░", "▒", "▒", "▒", "█",
			"▒", "▒", "▒", "█", "█",
			"▒", "█", "█", "█", "█"
		}},
		{"tty_down", {
			" ", "░", "░", "▒", "▒",
			"░", "░", "▒", "▒", "█",
			"░", "▒", "▒", "▒", "█",
			"▒", "▒", "▒", "█", "█",
			"▒", "█", "█", "█", "█"
		}}
	};

}

namespace Draw {

	TextEdit::TextEdit() {}
	TextEdit::TextEdit(string text) : text(text) {
		pos = this->text.size();
		upos = ulen(this->text);
	}

	bool TextEdit::command(const string& key) {
		if (key == "left" and upos > 0) {
			upos--;
			pos = uresize(text, upos).size();
		}
		else if (key == "right" and pos < text.size()) {
			upos++;
			pos = uresize(text, upos).size();
		}
		else if (key == "home" and pos > 0) {
			pos = upos = 0;
		}
		else if (key == "end" and pos < text.size()) {
			pos = text.size();
			upos = ulen(text);
		}
		else if (key == "backspace" and pos > 0) {
			if (pos == text.size()) {
				text = uresize(text, --upos);
				pos = text.size();
			}
			else {
				const string first = uresize(text, --upos);
				pos = first.size();
				text = first + text.substr(pos);
			}
		}
		else if (key == "delete" and pos < text.size()) {
			const string first = uresize(text, upos + 1);
			text = uresize(first, ulen(first) - 1) + text.substr(first.size());
		}
		else if (key == "space") {
			text.insert(pos++, 1, ' ');
			upos++;
		}
		else if (ulen(key) == 1) {
			if (key.size() == 1) {
				text.insert(pos++, 1, key[0]);
				upos++;
			}
			else {
				const string first = uresize(text, upos) + key;
				text = first + text.substr(pos);
				upos++;
				pos = first.size();
			}
		}
		else
			return false;

		return true;
	}

	string TextEdit::operator()(const size_t limit) {
		if (limit > 0 and ulen(text) + 1 > limit) {
			try {
				const size_t half = (size_t)round((double)limit / 2);
				string first;

				if (upos + half > ulen(text))
					first = luresize(text.substr(0, pos), limit - (ulen(text) - upos));
				else if (upos - half < 1)
					first = text.substr(0, pos);
				else
					first = luresize(text.substr(0, pos), half);

				return first + Fx::bl + "█" + Fx::ubl + uresize(text.substr(pos), limit - ulen(first));
			}
			catch (const std::exception& e) {
				Logger::error("In TextEdit::operator() : " + (string)e.what());
			}
		}
		return text.substr(0, pos) + Fx::bl + "█" + Fx::ubl + text.substr(pos);
	}

	string createBox(const int x, const int y, const int width, const int height, string line_color, const bool fill, const string title, const string title2, const int num) {
		string out;
		if (line_color.empty()) line_color = Theme::c("div_line");
		const auto& tty_mode = Config::getB("tty_mode");
		const string numbering = (num == 0) ? "" : Theme::c("hi_fg") + (tty_mode ? std::to_string(num) : Symbols::superscript[num]);
		const auto& right_up = (tty_mode ? Symbols::right_up : Symbols::round_right_up);
		const auto& left_up = (tty_mode ? Symbols::left_up : Symbols::round_left_up);
		const auto& right_down = (tty_mode ? Symbols::right_down : Symbols::round_right_down);
		const auto& left_down = (tty_mode ? Symbols::left_down : Symbols::round_left_down);

		out = Fx::reset + line_color;

		//? Draw horizontal lines
		for (const int& hpos : {y, y + height - 1}) {
			out += Mv::to(hpos, x) + Symbols::h_line * (width - 1);
		}

		//? Draw vertical lines and fill if enabled
		for (const int& hpos : iota(y + 1, y + height - 1)) {
			out += Mv::to(hpos, x) + Symbols::v_line
				+  ((fill) ? string(width - 2, ' ') : Mv::r(width - 2))
				+  Symbols::v_line;
		}

		//? Draw corners
		out += 	Mv::to(y, x) + left_up
			+	Mv::to(y, x + width - 1) + right_up
			+	Mv::to(y + height - 1, x) +left_down
			+	Mv::to(y + height - 1, x + width - 1) + right_down;

		//? Draw titles if defined
		if (not title.empty()) {
			out += Mv::to(y, x + 2) + Symbols::title_left + Fx::b + numbering + Theme::c("title") + title
				+  Fx::ub + line_color + Symbols::title_right;
		}
		if (not title2.empty()) {
			out += Mv::to(y + height - 1, x + 2) + Symbols::title_left_down + Fx::b + numbering + Theme::c("title") + title2
				+  Fx::ub + line_color + Symbols::title_right_down;
		}

		return out + Fx::reset + Mv::to(y + 1, x + 1);
	}

	//* Meter class ------------------------------------------------------------------------------------------------------------>
	Meter::Meter(const int width, const string& color_gradient, const bool invert) : width(width), color_gradient(color_gradient), invert(invert) {
		cache.insert(cache.begin(), 101, "");
	}

	string Meter::operator()(int value) {
		if (width < 1) return "";
		value = clamp(value, 0, 100);
		if (not cache.at(value).empty()) return cache.at(value);
		string& out = cache.at(value);
		for (const int& i : iota(1, width + 1)) {
			int y = round((double)i * 100.0 / width);
			if (value >= y)
				out += Theme::g(color_gradient)[invert ? 100 - y : y] + Symbols::meter;
			else {
				out += Theme::c("meter_bg") + Symbols::meter * (width + 1 - i);
				break;
			}
		}
		out += Fx::reset;
		return out;
	}

	//* Graph class ------------------------------------------------------------------------------------------------------------>
	void Graph::_create(const deque<long long>& data, int data_offset) {
		const bool mult = (data.size() - data_offset > 1);
		const auto& graph_symbol = Symbols::graph_symbols.at(symbol + '_' + (invert ? "down" : "up"));
		array<int, 2> result;
		const float mod = (height == 1) ? 0.3 : 0.1;
		long long data_value = 0;
		if (mult and data_offset > 0) {
			last = data[data_offset - 1];
			if (max_value > 0) last = clamp((last + offset) * 100 / max_value, 0ll, 100ll);
		}

		//? Horizontal iteration over values in <data>
		for (const int& i : iota(data_offset, (int)data.size())) {
			if (tty_mode and mult and i % 2 != 0) continue;
			else if (not tty_mode and mult) current = not current;
			if (i < 0) {
				data_value = 0;
				last = 0;
			}
			else {
				data_value = data[i];
				if (max_value > 0) data_value = clamp((data_value + offset) * 100 / max_value, 0ll, 100ll);
			}

			//? Vertical iteration over height of graph
			for (const int& horizon : iota(0, height)) {
				const int cur_high = (height > 1) ? round(100.0 * (height - horizon) / height) : 100;
				const int cur_low = (height > 1) ? round(100.0 * (height - (horizon + 1)) / height) : 0;
				const int clamp_min = (no_zero and horizon == height - 1 and i != -1) ? 1 : 0;
				//? Calculate previous + current value to fit two values in 1 braille character
				for (int ai = 0; const auto& value : {last, data_value}) {
					if (value >= cur_high)
						result[ai++] = 4;
					else if (value <= cur_low)
						result[ai++] = clamp_min;
					else {
						result[ai++] = clamp((int)round((float)(value - cur_low) * 4 / (cur_high - cur_low) + mod), clamp_min, 4);
					}
				}
				//? Generate graph symbol from 5x5 2D vector
				graphs[current][horizon] += (height == 1 and result[0] + result[1] == 0) ? Mv::r(1) : graph_symbol[(result[0] * 5 + result[1])];
			}
			if (mult and i >= 0) last = data_value;
		}
		last = data_value;
		out.clear();
		if (height == 1) {
			if (not color_gradient.empty())
				out += (last < 1 and not color_gradient.empty() ? Theme::c("inactive_fg") : Theme::g(color_gradient)[last]);
			out += graphs[current][0];
		}
		else {
			for (const int& i : iota(0, height)) {
				if (i > 0) out += Mv::d(1) + Mv::l(width);
				if (not color_gradient.empty())
					out += (invert) ? Theme::g(color_gradient)[i * 100 / (height - 1)] : Theme::g(color_gradient)[100 - (i * 100 / (height - 1))];
				out += (invert) ? graphs[current][ (height - 1) - i] : graphs[current][i];
			}
		}
		if (not color_gradient.empty()) out += Fx::reset;
	}

	Graph::Graph() {}

	Graph::Graph(int width, int height, const string& color_gradient, const deque<long long>& data, const string& symbol, bool invert, bool no_zero, long long max_value, long long offset)
	: width(width), height(height), color_gradient(color_gradient), invert(invert), no_zero(no_zero), offset(offset) {
		if (Config::getB("tty_mode") or symbol == "tty") { tty_mode = true; this->symbol = "tty"; }
		else if (symbol != "default") this->symbol = symbol;
		else this->symbol = Config::getS("graph_symbol");

		if (max_value == 0 and offset > 0) max_value = 100;
		this->max_value = max_value;
		const int value_width = ceil((double)data.size() / 2);
		int data_offset = (value_width > width) ? data.size() - width * 2 : 0;

		if ((data.size() - data_offset) % 2 != 0) {
			data_offset--;
		}

		//? Populate the two switching graph vectors and fill empty space if data size < width
		for (const int& i : iota(0, height * 2)) {
			if (tty_mode and i % 2 != current) continue;
			graphs[(i % 2 != 0)].push_back((value_width < width) ? ((height == 1) ? Mv::r(1) : " "s) * (width - value_width) : "");
		}
		if (data.size() == 0) return;
		this->_create(data, data_offset);
	}

	string& Graph::operator()(const deque<long long>& data, const bool data_same) {
		if (data_same) return out;

		//? Make room for new characters on graph
		if (not tty_mode) current = not current;
		for (const int& i : iota(0, height)) {
			if (graphs[current][i][1] == '[') graphs[current][i].erase(0, 4);
			else graphs[current][i].erase(0, 3);
		}
		this->_create(data, (int)data.size() - 1);
		return out;
	}

	string& Graph::operator()() {
		return out;
	}
	//*------------------------------------------------------------------------------------------------------------------------->

}

namespace Cpu {
	int width_p = 100, height_p = 32;
	int min_w = 60, min_h = 8;
	int x = 1, y = 1, width, height;
	int b_columns, b_column_size;
	int b_x, b_y, b_width, b_height;
	bool shown = true, redraw = true;
	string box;

	string draw(const cpu_info& cpu, const bool force_redraw, const bool data_same) {
		(void)data_same;
		string out;
		out.reserve(width * height);
		if (redraw or force_redraw) {
			auto& cpu_bottom = Config::getB("cpu_bottom");
			const int button_y = cpu_bottom ? y + height - 1 : y;
			out += box;
			const string title_left = Theme::c("cpu_box") + (cpu_bottom ? Symbols::title_left_down : Symbols::title_left);
			const string title_right = Theme::c("cpu_box") + (cpu_bottom ? Symbols::title_right_down : Symbols::title_right);
			out += Mv::to(button_y, x + 10) + title_left + Theme::c("hi_fg") + Fx::b + 'm' + Theme::c("title") + "enu" + Fx::ub + title_right;

		}

		if (Config::getB("show_cpu_freq") and not cpuHz.empty())
			out += Mv::to(b_y, b_x + b_width - 10) + Fx::ub + Theme::c("div_line") + Symbols::h_line * max(0ul, 7 - cpuHz.size())
				+ Symbols::title_left + Fx::b + Theme::c("title") + cpuHz + Fx::ub + Theme::c("div_line") + Symbols::title_right;


		//! VALS

		out += Mv::to(y + 2, x + 5) + Theme::c("title") + Fx::b + ljust("Cpu total=" + to_string(cpu.cpu_percent.at("total").back()), 20);
		out += Mv::to(y + 4, x + 5);
		for (int i = 0; const auto& cper : cpu.core_percent) { out += ljust("Core" + to_string(i++) + "=" + to_string(cper.back()), 10); }

		out += Mv::to(y + 6, x + 5);
		for (const auto& [name, value] : cpu.cpu_percent) { out += ljust(name + "=" + to_string(value.back()), 12); }


		redraw = false;
		return out;
	}

}

namespace Mem {
	int width_p = 45, height_p = 38;
	int min_w = 36, min_h = 10;
	int x = 1, y, width, height;
	int mem_width, disks_width, divider, item_height, mem_size, mem_meter, graph_height, disk_meter;
	bool shown = true, redraw = true;
	string box;

	string draw(const mem_info& mem, const bool force_redraw, const bool data_same) {
		(void)mem;
		(void)data_same;
		string out = Mv::to(0, 0);
		if (redraw or force_redraw) {
			redraw = false;
			out += box;
		}
		return out;
	}

}

namespace Net {
	int width_p = 45, height_p = 30;
	int min_w = 3, min_h = 6;
	int x = 1, y, width, height;
	int b_x, b_y, b_width, b_height, d_graph_height, u_graph_height;
	int graph_height;
	bool shown = true, redraw = true;
	string box;

	string draw(const net_info& net, const bool force_redraw, const bool data_same) {
		(void)net;
		(void)data_same;
		string out = Mv::to(0, 0);
		if (redraw or force_redraw) {
			redraw = false;
			out += box;
		}
		return out;
	}

}

namespace Proc {
	int width_p = 55, height_p = 68;
	int min_w = 44, min_h = 16;
	int x, y, width, height;
	int start, selected, select_max;
	bool shown = true, redraw = true;
	int selected_pid = 0;
	unordered_flat_map<size_t, Draw::Graph> p_graphs;
	unordered_flat_map<size_t, int> p_counters;
	int counter = 0;
	Draw::TextEdit filter;
	Draw::Graph detailed_cpu_graph;
	Draw::Graph detailed_mem_graph;
	int user_size, thread_size, prog_size, cmd_size, tree_size;
	int dgraph_x, dgraph_width, d_width, d_x, d_y;

	string box;

	int selection(const string& cmd_key) {
		auto start = Config::getI("proc_start");
		auto selected = Config::getI("proc_selected");
		auto last_selected = Config::getI("proc_last_selected");
		const int select_max = (Config::getB("show_detailed") ? Proc::select_max - 8 : Proc::select_max);

		int numpids = Proc::numpids;
		if (cmd_key == "up" and selected > 0) {
			if (start > 0 and selected == 1) start--;
			else selected--;
			if (Config::getI("proc_last_selected") > 0) Config::set("proc_last_selected", 0);
		}
		else if (cmd_key == "mouse_scroll_up" and start > 0) {
			start = max(0, start - 3);
		}
		else if (cmd_key == "mouse_scroll_down" and start < numpids - select_max) {
			start = min(numpids - select_max, start + 3);
		}
		else if (cmd_key == "down") {
			if (start < numpids - select_max and selected == select_max) start++;
			else if (selected == 0 and last_selected > 0) {
				selected = last_selected;
				Config::set("proc_last_selected", 0);
			}
			else selected++;
		}
		else if (cmd_key == "page_up") {
			if (selected > 0 and start == 0) selected = 0;
			else start = max(0, start - select_max);
		}
		else if (cmd_key == "page_down") {
			if (selected > 0 and start >= numpids - select_max) selected = select_max;
			else start = clamp(start + select_max, 0, max(0, numpids - select_max));
		}
		else if (cmd_key == "home") {
			start = 0;
			if (selected > 0) selected = 1;
		}
		else if (cmd_key == "end") {
			start = max(0, numpids - select_max);
			if (selected > 0) selected = select_max;
		}
		else if (cmd_key.starts_with("mousey")) {
			int mouse_y = std::stoi(cmd_key.substr(6));
			start = clamp((int)round((double)mouse_y * (numpids - select_max - 2) / (select_max - 2)), 0, max(0, numpids - select_max));
		}

		bool changed = false;
		if (start != Config::getI("proc_start")) {
			Config::set("proc_start", start);
			changed = true;
		}
		if (selected != Config::getI("proc_selected")) {
			Config::set("proc_selected", selected);
			changed = true;
		}
		return (not changed ? -1 : selected);
	}

	string draw(const vector<proc_info>& plist, const bool force_redraw, const bool data_same) {
		auto& proc_tree = Config::getB("proc_tree");
		const bool show_detailed = (Config::getB("show_detailed") and Proc::detailed.last_pid == (size_t)Config::getI("detailed_pid"));
		const bool proc_gradient = (Config::getB("proc_gradient") and not Config::getB("lowcolor") and Theme::gradients.contains("proc"));
		auto& proc_colors = Config::getB("proc_colors");
		const auto& tty_mode = Config::getB("tty_mode");
		const auto& graph_symbol = (tty_mode ? "tty" : Config::getS("graph_symbol_proc"));
		const auto& graph_bg = Symbols::graph_symbols.at((graph_symbol == "default" ? "braille_up" : graph_symbol + "_up"))[1];
		const auto& mem_bytes = Config::getB("proc_mem_bytes");
		start = Config::getI("proc_start");
		selected = Config::getI("proc_selected");
		uint64_t total_mem = 16328872 << 10;
		const int y = show_detailed ? Proc::y + 8 : Proc::y;
		const int height = show_detailed ? Proc::height - 8 : Proc::height;
		const int select_max = show_detailed ? Proc::select_max - 8 : Proc::select_max;
		int numpids = Proc::numpids;
		if (force_redraw) redraw = true;
		string out;
		out.reserve(width * height);

		//* Redraw elements not needed to be updated every cycle
		if (redraw) {
			out = box;
			const string title_left = Theme::c("proc_box") + Symbols::title_left;
			const string title_right = Theme::c("proc_box") + Symbols::title_right;
			const string title_left_down = Theme::c("proc_box") + Symbols::title_left_down;
			const string title_right_down = Theme::c("proc_box") + Symbols::title_right_down;
			for (const auto& key : {"T", "K", "S", "enter"})
				if (Input::mouse_mappings.contains(key)) Input::mouse_mappings.erase(key);

			//? Adapt sizes of text fields
			user_size = (width < 75 ? 5 : 10);
			thread_size = (width < 75 ? - 1 : 4);
			prog_size = (width > 70 ? 16 : ( width > 55 ? 8 : width - user_size - thread_size - 33));
			cmd_size = (width > 55 ? width - prog_size - user_size - thread_size - 33 : -1);
			tree_size = width - user_size - thread_size - 23;

			//? Detailed box
			if (show_detailed) {
				const bool alive = detailed.status != "Dead";
				dgraph_x = x;
				dgraph_width = max(width / 3, width - 121);
				d_width = width - dgraph_width - 1;
				d_x = x + dgraph_width + 1;
				d_y = Proc::y;

				//? Create cpu and mem graphs if process is alive
				if (alive) {
					detailed_cpu_graph = {dgraph_width - 1, 7, "cpu", detailed.cpu_percent, graph_symbol};
					detailed_mem_graph = {d_width / 3, 1, "", detailed.mem_bytes, graph_symbol, false, false, detailed.first_mem};
				}

				//? Draw structure of details box
				const string pid_str = to_string(detailed.entry.pid);
				out += Mv::to(y, x) + Theme::c("proc_box") + Symbols::div_left + Symbols::h_line + title_left + Theme::c("hi_fg") + Fx::b
				+ (tty_mode ? "4" : Symbols::superscript[4]) + Theme::c("title") + "proc"
					+ Fx::ub + title_right + Symbols::h_line * (width - 10) + Symbols::div_right
					+ Mv::to(d_y, dgraph_x + 2) + title_left + Fx::b + Theme::c("title") + pid_str + Fx::ub + title_right
					+ title_left + Fx::b + Theme::c("title") + uresize(detailed.entry.name, dgraph_width - pid_str.size() - 7, true) + Fx::ub + title_right;

				out += Mv::to(d_y, d_x - 1) + Theme::c("proc_box") + Symbols::div_up + Mv::to(y, d_x - 1) + Symbols::div_down + Theme::c("div_line");
				for (const int& i : iota(1, 8)) out += Mv::to(d_y + i, d_x - 1) + Symbols::v_line;

				const string& t_color = (not alive or selected > 0 ? Theme::c("inactive_fg") : Theme::c("title"));
				const string& hi_color = (not alive or selected > 0 ? t_color : Theme::c("hi_fg"));
				const string hide = (selected > 0 ? t_color + "hide " : Theme::c("title") + "hide " + Theme::c("hi_fg"));
				int mouse_x = d_x + 2;
				out += Mv::to(d_y, d_x + 1);
				if (width > 55) {
					out += title_left + hi_color + Fx::b + 't' + t_color + "erminate" + Fx::ub + title_right;
					if (alive and selected == 0) Input::mouse_mappings["t"] = {d_y, mouse_x, 1, 9};
					mouse_x += 11;
				}
				out += title_left + hi_color + Fx::b + 'k' + t_color + "ill" + Fx::ub + title_right
					+ title_left + hi_color + Fx::b + 's' + t_color + "ignals" + Fx::ub + title_right
					+ Mv::to(d_y, d_x + d_width - 10) + title_left + t_color + Fx::b + hide + Symbols::enter + Fx::ub + title_right;
				if (alive and selected == 0) {
					Input::mouse_mappings["k"] = {d_y, mouse_x, 1, 4};
					mouse_x += 6;
					Input::mouse_mappings["s"] = {d_y, mouse_x, 1, 7};
				}
				if (selected == 0) Input::mouse_mappings["enter"] = {d_y, d_x + d_width - 9, 1, 6};

				//? Labels
				const int item_fit = floor((double)(d_width - 2) / 10);
				const int item_width = floor((double)(d_width - 2) / min(item_fit, 8));
				out += Mv::to(d_y + 1, d_x + 1) + Fx::b + Theme::c("title")
										+ cjust("Status:", item_width)
										+ cjust("Elapsed:", item_width);
				if (item_fit >= 3) out += cjust("IO/R:", item_width);
				if (item_fit >= 4) out += cjust("IO/W:", item_width);
				if (item_fit >= 5) out += cjust("Parent:", item_width);
				if (item_fit >= 6) out += cjust("User:", item_width);
				if (item_fit >= 7) out += cjust("Nice:", item_width);
				if (item_fit >= 8) out += cjust("Threads:", item_width);

				//? Command line
				for (int i = 0; const auto& l : {'C', 'M', 'D'})
				out += Mv::to(d_y + 5 + i++, d_x + 1) + l;

				out += Theme::c("main_fg") + Fx::ub;
				const int cmd_size = ulen(detailed.entry.cmd, true);
				for (int num_lines = min(3, (int)ceil((double)cmd_size / (d_width - 5))), i = 0; i < num_lines; i++) {
					out += Mv::to(d_y + 5 + (num_lines == 1 ? 1 : i), d_x + 3)
						+ cjust(luresize(detailed.entry.cmd, cmd_size - (d_width - 5) * i, true), d_width - 5, true, true);
				}

			}

			//? Filter
			const auto& filtering = Config::getB("proc_filtering"); // ? filter(20) : Config::getS("proc_filter"))
			const auto filter_text = (filtering) ? filter(max(6, width - 58)) : uresize(Config::getS("proc_filter"), max(6, width - 58));
			out += Mv::to(y, x+9) + title_left + (not filter_text.empty() ? Fx::b : "") + Theme::c("hi_fg") + 'f'
				+ Theme::c("title") + (not filter_text.empty() ? ' ' + filter_text : "ilter")
				+ (not filtering and not filter_text.empty() ? Theme::c("hi_fg") + " del" : "")
				+ (filtering ? Theme::c("hi_fg") + ' ' + Symbols::enter : "") + Fx::ub + title_right;
			if (not filtering) {
				int f_len = (filter_text.empty() ? 6 : ulen(filter_text) + 2);
				Input::mouse_mappings["f"] = {y, x + 10, 1, f_len};
				if (filter_text.empty() and Input::mouse_mappings.contains("delete"))
					Input::mouse_mappings.erase("delete");
				else if (not filter_text.empty())
					Input::mouse_mappings["delete"] = {y, x + 11 + f_len, 1, 3};
			}

			//? per-core, reverse, tree and sorting
			const auto& sorting = Config::getS("proc_sorting");
			const int sort_len = sorting.size();
			const int sort_pos = x + width - sort_len - 8;

			if (width > 55 + sort_len) {
				out += Mv::to(y, sort_pos - 25) + title_left + (Config::getB("proc_per_core") ? Fx::b : "") + Theme::c("title")
					+ "per-" + Theme::c("hi_fg") + 'c' + Theme::c("title") + "ore" + Fx::ub + title_right;
				Input::mouse_mappings["c"] = {y, sort_pos - 24, 1, 8};
			}
			if (width > 45 + sort_len) {
				out += Mv::to(y, sort_pos - 15) + title_left + (Config::getB("proc_reversed") ? Fx::b : "") + Theme::c("hi_fg")
					+ 'r' + Theme::c("title") + "everse" + Fx::ub + title_right;
				Input::mouse_mappings["r"] = {y, sort_pos - 14, 1, 7};
			}
			if (width > 35 + sort_len) {
				out += Mv::to(y, sort_pos - 6) + title_left + (Config::getB("proc_tree") ? Fx::b : "") + Theme::c("title") + "tre"
					+ Theme::c("hi_fg") + 'e' + Fx::ub + title_right;
				Input::mouse_mappings["e"] = {y, sort_pos - 5, 1, 4};
			}
			out += Mv::to(y, sort_pos) + title_left + Fx::b + Theme::c("hi_fg") + "< " + Theme::c("title") + sorting + Theme::c("hi_fg")
				+ " >" + Fx::ub + title_right;
				Input::mouse_mappings["left"] = {y, sort_pos + 1, 1, 2};
				Input::mouse_mappings["right"] = {y, sort_pos + sort_len + 3, 1, 2};

			//? select, info and signal buttons
			const string down_button = (selected == select_max and start == numpids - select_max ? Theme::c("inactive_fg") : Theme::c("hi_fg")) + Symbols::down;
			const string t_color = (selected == 0 ? Theme::c("inactive_fg") : Theme::c("title"));
			const string hi_color = (selected == 0 ? Theme::c("inactive_fg") : Theme::c("hi_fg"));
			int mouse_x = x + 14;
			out += Mv::to(y + height - 1, x + 1) + title_left_down + Fx::b + hi_color + Symbols::up + Theme::c("title") + " select " + down_button + Fx::ub + title_right_down
				+ title_left_down + Fx::b + t_color + "info " + hi_color + Symbols::enter + Fx::ub + title_right_down;
				if (selected > 0) Input::mouse_mappings["enter"] = {y + height - 1, mouse_x, 1, 6};
				mouse_x += 8;
			if (width > 60) {
				out += title_left_down + Fx::b + hi_color + 't' + t_color + "erminate" + Fx::ub + title_right_down;
				if (selected > 0) Input::mouse_mappings["t"] = {y + height - 1, mouse_x, 1, 9};
				mouse_x += 11;
			}
			if (width > 55) {
				out += title_left_down + Fx::b + hi_color + 'k' + t_color + "ill" + Fx::ub + title_right_down;
				if (selected > 0) Input::mouse_mappings["k"] = {y + height - 1, mouse_x, 1, 4};
				mouse_x += 6;
			}
			out += title_left_down + Fx::b + hi_color + 's' + t_color + "ignals" + Fx::ub + title_right_down;
			if (selected > 0) Input::mouse_mappings["s"] = {y + height - 1, mouse_x, 1, 7};

			//? Labels for fields in list
			if (not proc_tree)
				out += Mv::to(y+1, x+1) + Theme::c("title") + Fx::b
					+ rjust("Pid:", 8) + ' '
					+ ljust("Program:", prog_size) + ' '
					+ (cmd_size > 0 ? ljust("Command:", cmd_size) : "") + ' ';
			else
				out += Mv::to(y+1, x+1) + Theme::c("title") + Fx::b
					+ ljust("Tree:", tree_size) + ' ';

			out += (thread_size > 0 ? Mv::l(4) + "Threads: " : "")
					+ ljust("User:", user_size) + ' '
					+ rjust((mem_bytes ? "MemB" : "Mem%"), 5) + ' '
					+ rjust("Cpu%", 10) + Fx::ub;
		}
		//* End of redraw block

		//? Draw details box if shown
		if (show_detailed) {
			const bool alive = detailed.status != "Dead";
			const int item_fit = floor((double)(d_width - 2) / 10);
			const int item_width = floor((double)(d_width - 2) / min(item_fit, 8));

			//? Graph part of box
			string cpu_str = (alive ? to_string(detailed.entry.cpu_p) : "");
			if (alive) {
				cpu_str.resize((detailed.entry.cpu_p < 10 or detailed.entry.cpu_p >= 100 ? 3 : 4));
				cpu_str += '%' + Mv::r(1) + (dgraph_width < 20 ? "C" : "Core") + to_string(detailed.entry.cpu_n + 1);
			}
			out += Mv::to(d_y + 1, dgraph_x + 1) + Fx::ub + detailed_cpu_graph(detailed.cpu_percent, (redraw or data_same or not alive))
				+ Mv::to(d_y + 1, dgraph_x + 1) + Theme::c("title") + Fx::b + cpu_str;
			for (int i = 0; const auto& l : {'C', 'P', 'U'})
					out += Mv::to(d_y + 3 + i++, dgraph_x + 1) + l;

			//? Info part of box
			const string stat_color = (not alive ? Theme::c("inactive_fg") : (detailed.status == "Running" ? Theme::c("proc_misc") : Theme::c("main_fg")));
			out += Mv::to(d_y + 2, d_x + 1) + stat_color + Fx::ub
									+ cjust(detailed.status, item_width) + Theme::c("main_fg")
									+ cjust(detailed.elapsed, item_width);
			if (item_fit >= 3) out += cjust(detailed.io_read, item_width);
			if (item_fit >= 4) out += cjust(detailed.io_write, item_width);
			if (item_fit >= 5) out += cjust(detailed.parent, item_width, true);
			if (item_fit >= 6) out += cjust(detailed.entry.user, item_width, true);
			if (item_fit >= 7) out += cjust(to_string(detailed.entry.p_nice), item_width);
			if (item_fit >= 8) out += cjust(to_string(detailed.entry.threads), item_width);

			const double mem_p = (double)detailed.mem_bytes.back() * 100 / Shared::totalMem;
			string mem_str = to_string(mem_p);
			mem_str.resize((mem_p < 10 or mem_p >= 100 ? 3 : 4));
			out += Mv::to(d_y + 4, d_x + 1) + Theme::c("title") + Fx::b + rjust((item_fit > 4 ? "Memory: " : "M:") + mem_str + "% ", (d_width / 3) - 2)
				+ Theme::c("inactive_fg") + Fx::ub + graph_bg * (d_width / 3) + Mv::l(d_width / 3)
				+ Theme::c("proc_misc") + detailed_mem_graph(detailed.mem_bytes, (redraw or data_same or not alive)) + ' '
				+ Theme::c("title") + Fx::b + detailed.memory;


		}


		//? Check bounds of current selection and view
		if (start > 0 and numpids <= select_max)
			start = 0;
		if (start > numpids - select_max)
			start = max(0, numpids - select_max);
		if (selected > select_max)
			selected = select_max;
		if (selected > numpids)
			selected = numpids;

		//* Iteration over processes
		int lc = 0;
		for (int n=0; auto& p : plist) {
			if (n++ < start) continue;
			bool is_selected = (lc + 1 == selected);
			if (is_selected) selected_pid = (int)p.pid;

			//? Update graphs for processes with above 0.0% cpu usage, delete if below 0.1% 10x times
			const bool has_graph = p_counters.contains(p.pid);
			if ((p.cpu_p > 0 and not has_graph) or (not data_same and has_graph)) {
				if (not has_graph) {
					p_graphs[p.pid] = {5, 1, "", {}, graph_symbol};
					p_counters[p.pid] = 0;
				}
				else if (p.cpu_p < 0.1 and ++p_counters[p.pid] >= 10) {
					p_graphs.erase(p.pid);
					p_counters.erase(p.pid);
				}
				else
					p_counters[p.pid] = 0;
			}

			out += Fx::reset;

			//? Set correct gradient colors if enabled
			string c_color, m_color, t_color, g_color, end;
			if (is_selected) {
				c_color = m_color = t_color = g_color = Fx::b;
				end = Fx::ub;
				out += Theme::c("selected_bg") + Theme::c("selected_fg") + Fx::b;
			}
			else {
				int calc = (selected > lc) ? selected - lc : lc - selected;
				if (proc_colors) {
					end = Theme::c("main_fg") + Fx::ub;
					array<string, 3> colors;
					for (int i = 0; int v : {(int)round(p.cpu_p), (int)round(p.mem * 100 / total_mem), (int)p.threads / 3}) {
						if (proc_gradient) {
							int val = (min(v, 100) + 100) - calc * 100 / select_max;
							if (val < 100) colors[i++] = Theme::g("proc_color")[val];
							else colors[i++] = Theme::g("process")[val - 100];
						}
						else
							colors[i++] = Theme::g("process")[min(v, 100)];
					}
					c_color = colors[0]; m_color = colors[1]; t_color = colors[2];
				}
				else {
					c_color = m_color = t_color = Fx::b;
					end = Fx::ub;
				}
				if (proc_gradient) {
					g_color = Theme::g("proc")[calc * 100 / select_max];
				}
			}

			//? Normal view line
			if (not proc_tree) {
				out += Mv::to(y+2+lc, x+1)
					+ g_color + rjust(to_string(p.pid), 8) + ' '
					+ c_color + ljust(p.name, prog_size, true) + ' ' + end
					+ (cmd_size > 0 ? g_color + ljust(p.cmd, cmd_size, true, true) + ' ' : "");
			}
			//? Tree view line
			else {
				string prefix_pid = p.prefix + to_string(p.pid);
				int width_left = tree_size;
				out += Mv::to(y+2+lc, x+1) + g_color + uresize(prefix_pid, width_left) + ' ';
				width_left -= ulen(prefix_pid);
				if (width_left > 0) {
					out += c_color + uresize(p.name, width_left - 1) + end + ' ';
					width_left -= (ulen(p.name) + 1);
				}
				if (width_left > 7 and not p.cmd.empty()) {
					out += g_color + uresize(p.cmd, width_left - 1, true) + ' ';
					width_left -= (ulen(p.cmd, true) + 1);
				}
				out += string(max(0, width_left), ' ');
			}
			//? Common end of line
			string cpu_str = to_string(p.cpu_p);
			if (p.cpu_p < 10 or p.cpu_p >= 100) cpu_str.resize(3);
			string mem_str = (mem_bytes ? floating_humanizer(p.mem, true) : "");
			if (not mem_bytes) {
				double mem_p = (double)p.mem * 100 / Shared::totalMem;
				mem_str = to_string(mem_p);
				mem_str.resize((mem_p < 10 or mem_p >= 100 ? 3 : 4));
				mem_str += '%';
			}
			out += (thread_size > 0 ? t_color + rjust(to_string(min(p.threads, 9999ul)), thread_size) + ' ' + end : "" )
				+ g_color + ljust(((int)p.user.size() > user_size ? p.user.substr(0, user_size - 1) + '+' : p.user), user_size) + ' '
				+ m_color + rjust(mem_str, 5) + end + ' '
				+ (is_selected ? "" : Theme::c("inactive_fg")) + graph_bg * 5
				+ (p_graphs.contains(p.pid) ? Mv::l(5) + c_color + p_graphs[p.pid]({(p.cpu_p >= 0.1 and p.cpu_p < 5 ? 5ll : (long long)round(p.cpu_p))}, data_same) : "") + end + ' '
				+ c_color + rjust(cpu_str, 4) + "  " + end;
			if (lc++ > height - 5) break;
		}

		out += Fx::reset;
		while (lc++ < height - 4) out += Mv::to(y+lc+2, x+1) + string(width - 2, ' ');

		//? Draw scrollbar if needed
		if (numpids > select_max) {
			const int scroll_pos = clamp((int)round((double)start * (select_max - 2) / (numpids - (select_max - 2))), 0, height - 5);
			out += Mv::to(y + 1, x + width - 2) + Fx::b + Theme::c("main_fg") + Symbols::up
				+ Mv::to(y + height - 2, x + width - 2) + Symbols::down
				+ Mv::to(y + 2 + scroll_pos, x + width - 2) + "█";
		}

		//? Current selection and number of processes
		string location = to_string(start + selected) + '/' + to_string(numpids);
		string loc_clear = Symbols::h_line * max(0ul, 9 - location.size());
		out += Mv::to(y + height - 1, x+width - 3 - max(9, (int)location.size())) + Theme::c("proc_box") + loc_clear
			+ Symbols::title_left_down + Theme::c("title") + Fx::b + location + Fx::ub + Theme::c("proc_box") + Symbols::title_right_down;

		//? Clear out left over graphs from dead processes at a regular interval
		if (not data_same and ++counter >= 1000) {
			counter = 0;
			for (auto element = p_graphs.begin(); element != p_graphs.end();) {
				if (rng::find(plist, element->first, &proc_info::pid) == plist.end()) {
					element = p_graphs.erase(element);
					p_counters.erase(element->first);
				}
				else
					++element;
			}
		}

		if (selected == 0 and selected_pid != 0) selected_pid = 0;
		redraw = false;
		return out + Fx::reset;
	}

}

namespace Draw {
	void calcSizes() {
		auto& boxes = Config::getS("shown_boxes");
		auto& cpu_bottom = Config::getB("cpu_bottom");
		auto& mem_below_net = Config::getB("mem_below_net");
		auto& proc_left = Config::getB("proc_left");

		Cpu::box.clear();
		Mem::box.clear();
		Net::box.clear();
		Proc::box.clear();

		Input::mouse_mappings.clear();

		Cpu::x = Mem::x = Net::x = Proc::x = 1;
		Cpu::y = Mem::y = Net::y = Proc::y = 1;
		Cpu::width = Mem::width = Net::width = Proc::width = 0;
		Cpu::height = Mem::height = Net::height = Proc::height = 0;
		Cpu::redraw = Mem::redraw = Net::redraw = Proc::redraw = true;

		Cpu::shown = s_contains(boxes, "cpu");
		Mem::shown = s_contains(boxes, "mem");
		Net::shown = s_contains(boxes, "net");
		Proc::shown = s_contains(boxes, "proc");

		//* Calculate and draw cpu box outlines
		if (Cpu::shown) {
			using namespace Cpu;
			const bool show_temp = (Config::getB("check_temp") and got_sensors);
			width = round((double)Term::width * width_p / 100);
			height = max(8, (int)round((double)Term::height * (trim(boxes) == "cpu" ? 100 : height_p) / 100));
			x = 1;
			y = cpu_bottom ? Term::height - height + 1 : 1;

			b_columns = max(1, (int)ceil((double)(Shared::coreCount + 1) / (height - 5)));
			if (b_columns * (21 + 12 * show_temp) < width - (width / 3)) {
				b_column_size = 2;
				b_width = (21 + 12 * show_temp) * b_columns - (b_columns - 1);
			}
			else if (b_columns * (15 + 6 * show_temp) < width - (width / 3)) {
				b_column_size = 1;
				b_width = (15 + 6 * show_temp) * b_columns - (b_columns - 1);
			}
			else if (b_columns * (8 + 6 * show_temp) < width - (width / 3)) {
				b_column_size = 0;
			}
			else {
				b_columns = (width - width / 3) / (8 + 6 * show_temp);
				b_column_size = 0;
			}

			if (b_column_size == 0) b_width = (8 + 6 * show_temp) * b_columns + 1;
			b_height = min(height - 2, (int)ceil((double)Shared::coreCount / b_columns) + 4);

			b_x = x + width - b_width - 1;
			b_y = y + ceil((double)(height - 2) / 2) - ceil((double)b_height / 2) + 1;

			box = createBox(x, y, width, height, Theme::c("cpu_box"), true, (cpu_bottom ? "" : "cpu"), (cpu_bottom ? "cpu" : ""), 1);

			auto& custom = Config::getS("custom_cpu_name");
			const string cpu_title = uresize((custom.empty() ? Cpu::cpuName : custom) , b_width - 14);
			box += createBox(b_x, b_y, b_width, b_height, "", false, cpu_title);
		}

		//* Calculate and draw mem box outlines
		if (Mem::shown) {
			using namespace Mem;
			auto& show_disks = Config::getB("show_disks");
			auto& swap_disk = Config::getB("swap_disk");
			auto& mem_graphs = Config::getB("mem_graphs");

			width = round((double)Term::width * (Proc::shown ? width_p : 100) / 100);
			height = round((double)Term::height * (100 - Cpu::height_p * Cpu::shown - Net::height_p * Net::shown) / 100) + 1;
			if (height + Cpu::height > Term::height) height = Term::height - Cpu::height;
			x = (proc_left and Proc::shown) ? Term::width - width + 1: 1;
			if (mem_below_net and Net::shown)
				y = Term::height - height + 1 - (cpu_bottom ? Cpu::height : 0);
			else
				y = cpu_bottom ? 1 : Cpu::height + 1;

			if (show_disks) {
				mem_width = ceil((double)(width - 3) / 2);
				disks_width = width - mem_width - 3;
				mem_width += mem_width % 2;
				divider = x + mem_width;
			}
			else
				mem_width = width - 1;

			item_height = has_swap and not swap_disk ? 6 : 4;
			if (height - (has_swap and not swap_disk ? 3 : 2) > 2 * item_height)
				mem_size = 3;
			else if (mem_width > 25)
				mem_size = 2;
			else
				mem_size = 1;

			mem_meter = max(0, width - (disks_width * show_disks) - (mem_size > 2 ? 9 : 20));
			if (mem_size == 1) mem_meter += 6;

			if (mem_graphs) {
				graph_height = max(1, (int)round((double)((height - (has_swap and not swap_disk ? 2 : 1)) - (mem_size == 3 ? 2 : 1) * item_height) / item_height));
				if (graph_height > 1) mem_meter += 6;
			}
			else
				graph_height = 0;

			if (show_disks) {
				disk_meter = max(0, width - mem_width - 23);
				if (disks_width < 25) disk_meter += 10;
			}

			box = createBox(x, y, width, height, Theme::c("mem_box"), true, "mem", "", 2);
			box += Mv::to(y, (show_disks ? divider + 2 : x + width - 9)) + Theme::c("mem_box") + Symbols::title_left + (show_disks ? Fx::b : "")
				+ Theme::c("hi_fg") + 'd' + Theme::c("title") + "isks" + Fx::ub + Theme::c("mem_box") + Symbols::title_right;
			if (show_disks) {
				box += Mv::to(y, divider) + Symbols::div_up + Mv::to(y + height - 1, divider) + Symbols::div_down + Theme::c("div_line");
				for (auto i : iota(1, height - 1))
					box += Mv::to(y + i, divider) + Symbols::v_line;
			}
		}

		//* Calculate and draw net box outlines
		if (Net::shown) {
			using namespace Net;
			width = round((double)Term::width * (Proc::shown ? width_p : 100) / 100);
			height = Term::height - Cpu::height - Mem::height;
			x = (proc_left and Proc::shown) ? Term::width - width + 1 : 1;
			if (mem_below_net and Mem::shown)
				y = cpu_bottom ? 1 : Cpu::height + 1;
			else
				y = Term::height - height + 1 - (cpu_bottom ? Cpu::height : 0);

			b_width = (width > 45) ? 27 : 19;
			b_height = (height > 10) ? 9 : height - 2;
			b_x = x + width - b_width - 1;
			b_y = y + ((height - 2) / 2) - b_height / 2 + 1;
			d_graph_height = round((double)(height - 2) / 2);
			u_graph_height = height - 2 - d_graph_height;

			box = createBox(x, y, width, height, Theme::c("net_box"), true, "net", "", 3);
			box += createBox(b_x, b_y, b_width, b_height, "", false, "download", "upload");
		}

		//* Calculate and draw proc box outlines
		if (Proc::shown) {
			using namespace Proc;
			width = Term::width - (Mem::shown ? Mem::width : (Net::shown ? Net::width : 0));
			height = Term::height - Cpu::height;
			x = proc_left ? 1 : Term::width - width + 1;
			y = (cpu_bottom and Cpu::shown) ? 1 : Cpu::height + 1;
			select_max = height - 3;
			box = createBox(x, y, width, height, Theme::c("proc_box"), true, "proc", "", 4);
		}

	}
}