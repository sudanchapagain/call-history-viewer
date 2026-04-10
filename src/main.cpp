#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tinyxml2.h>

static int PAGE_SIZE = 50;

struct Call {
    std::string number;
    std::string duration;
    std::string readable_date;
    std::string type;
    std::string contact_name;
};

static const std::vector<std::string>
HEADERS = {
    "#", "number", "duration", "date", "type", "contact name"
};


std::string
decode_type(const std::string& t) {
    if (t == "1") return "incoming";
    if (t == "2") return "outgoing";
    if (t == "3") return "missed";
    return "unknown";
}


std::vector<Call>
parse_file(const std::string& path) {
    std::vector<Call> calls;
    tinyxml2::XMLDocument doc;

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "failed to parse xml: " << doc.ErrorStr() << "\n";
        return calls;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) {
        std::cerr << "empty xml document\n";
        return calls;
    }

    for (
        auto* el = root->FirstChildElement("call");
        el;
        el = el->NextSiblingElement("call")
    ) {
        auto attr = [&](const char* name) -> std::string {
            const char* v = el->Attribute(name);
            return v ? v : "";
        };

        calls.push_back({
            attr("number"),
            attr("duration"),
            attr("readable_date"),
            decode_type(attr("type")),
            attr("contact_name"),
        });
    }
    return calls;
}


void
display_page(
    const std::vector<Call>& data,
    int page
) {
    std::cout << "\033[2J\033[H";

    int total = (int)data.size();
    int total_pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    int start = page * PAGE_SIZE;
    int end   = std::min(start + PAGE_SIZE, total);

    using Row = std::vector<std::string>;
    std::vector<Row> rows;
    for (int i = start; i < end; ++i) {
        const Call& c = data[i];
        rows.push_back({
            std::to_string(i),
            c.number, c.duration, c.readable_date, c.type, c.contact_name
        });
    }

    std::vector<int> widths(HEADERS.size());
    for (int i = 0; i < (int)HEADERS.size(); ++i)
        widths[i] = (int)HEADERS[i].size() + 2;

    for (auto& row : rows)
        for (int i = 0; i < (int)row.size(); ++i)
            widths[i] = std::max(widths[i], (int)row[i].size() + 2);

    auto repeat_dash = [](int n) {
        std::string s;
        s.reserve(n * 3);
        for (int i = 0; i < n; ++i) s += "─";
        return s;
    };

    auto border = [&](
        const std::string& l,
        const std::string& m,
        const std::string& r
    ) {
        std::string s = l;
        for (int i = 0; i < (int)widths.size(); ++i) {
            s += repeat_dash(widths[i]);
            if (i + 1 < (int)widths.size())
                s += m;
        }
        s += r;
        return s;
    };

    auto draw_row = [&](const Row& row) {
        std::string s = "│";
        for (int i = 0; i < (int)row.size(); ++i) {
            s += " ";
            s += row[i];
            s += std::string(widths[i] - 1 - (int)row[i].size(), ' ');
            s += "│";
        }
        return s;
    };

    std::cout << border("╭", "┬", "╮") << "\n";
    std::cout << draw_row(HEADERS) << "\n";
    std::cout << border("├", "┼", "┤") << "\n";
    for (auto& row : rows)
        std::cout << draw_row(row) << "\n";
    std::cout << border("╰", "┴", "╯") << "\n";

    std::cout << "page " << (page + 1) << " of " << total_pages << "\n\n";
}


std::vector<Call>
search_calls(
    const std::vector<Call>& data,
    const std::string& term
) {
    std::string t = term;
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);

    std::vector<Call> result;
    for (auto& c : data) {
        std::string all = c.number + " "
                        + c.duration + " "
                        + c.readable_date + " "
                        + c.type + " "
                        + c.contact_name;

        std::transform(all.begin(), all.end(), all.begin(), ::tolower);
        if (all.find(t) != std::string::npos)
            result.push_back(c);
    }
    return result;
}


std::vector<Call>
sort_calls(std::vector<Call> data) {
    std::cout <<
        "sort by: number, duration, readable_date, type, contact_name\n";

    std::cout << "key: ";

    std::string key;
    std::cin >> key;
    std::cin.ignore();

    auto by = [&](const Call& a, const Call& b) -> bool {
        auto lower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };

        if (key == "number")
            return lower(a.number) < lower(b.number);

        if (key == "duration") {
            int da = a.duration.empty() ? 0 : std::stoi(a.duration);
            int db = b.duration.empty() ? 0 : std::stoi(b.duration);
            return da < db;
        }

        if (key == "readable_date")
            return lower(a.readable_date) < lower(b.readable_date);

        if (key == "type")
            return lower(a.type) < lower(b.type);

        if (key == "contact_name")
            return lower(a.contact_name) < lower(b.contact_name);

        return false;
    };

    if (key == "number"
        || key == "duration"
        || key == "readable_date"
        || key == "type"
        || key == "contact_name"
    ) {
        std::sort(data.begin(), data.end(), by);
    } else {
        std::cout << "invalid key\n";
    }

    return data;
}


void
export_to_csv(
    const std::vector<Call>& data,
    const std::string& filename = "export.csv"
) {
    std::ofstream f(filename);
    if (!f) {
        std::cerr << "failed to open " << filename << "\n";
        return;
    }

    f << "number,duration,readable_date,type,contact_name\n";
    for (auto& c : data) {
        f << c.number << ","
          << c.duration << ","
          << c.readable_date << ","
          << c.type << ","
          << c.contact_name << "\n";
    }

    std::cout << "exported to " << filename << "\n";
}


std::string
prompt_file() {
    while (true) {
        std::cout << "enter path to xml file: ";
        std::string path;
        std::getline(std::cin, path);

        if (path.empty()) std::cout << "empty path\n";
        else if (path.size() < 4 || path.substr(path.size() - 4) != ".xml") {
            std::cout << "file must be .xml\n";
        } else {
            std::ifstream f(path);
            if (!f) {
                std::cout << "file does not exist\n";
                continue;
            }
            return path;
        }
    }
}


void
run_ui(const std::vector<Call>& data) {
    std::vector<Call> filtered = data;
    int page = 0;

    while (true) {
        display_page(filtered, page);
        std::cout << "[n]ext [p]rev [s]earch [r]eset [o]rder [e]xport [q]uit\n> ";

        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd.empty()) continue;
        char c = cmd[0];

        if (c == 'n') {
            if ((page + 1) * PAGE_SIZE < (int)filtered.size()) page++;
        } else if (c == 'p') {
            if (page > 0) page--;
        } else if (c == 's') {
            std::cout << "search term: ";
            std::string term;
            std::getline(std::cin, term);
            if (!term.empty()) {
                filtered = search_calls(data, term);
                page = 0;
            }
        } else if (c == 'r') {
            filtered = data;
            page = 0;
        } else if (c == 'o') {
            filtered = sort_calls(filtered);
            page = 0;
        } else if (c == 'e') {
            export_to_csv(filtered);
        } else if (c == 'q') {
            std::cout << "goodbye\n";
            break;
        } else {
            std::cout << "unknown command\n";
        }
    }
}


int
main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--page_size=", 0) == 0)
            PAGE_SIZE = std::stoi(arg.substr(12));
    }

    std::cout << "call history viewer (xml)\n\n";

    std::string path = prompt_file();
    std::vector<Call> data = parse_file(path);

    if (data.empty()) {
        std::cout << "no data found\n";
        return 0;
    }

    run_ui(data);
    return 0;
}
