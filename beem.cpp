#include <iostream>
#include<algorithm>
#include<vector>
#include<queue>
#include<map>
#include<time.h>

#include<cassert>


using namespace std;

int gCount = 0;
multimap<int, pair<int, int>> node;

#define be_size 50
#define manhattan 1
#define pow2manhattan 1
#define ntw 0
#define pow2ntw 0
#define wro 1

char rot[257];//回転情報の保存

int parity(vector<int> board, int size);

class set {
public:
    queue<int> moved;       //今までの操作の保持
    vector<int> board;      //現在の盤面
    int val;                //現在の盤面の評価値
    int selectable;         //現在の残り選択可能回数
    int n_select;           //現在選択しているピースの"座標"
    int recent;             //遷移元で行った交換操作の交換方向

    set& operator =(const set& temp) {
        moved = temp.moved;
        board = temp.board;
        val = temp.val;
        selectable = temp.selectable;
        n_select = temp.n_select;
        recent = temp.recent;
        return *this;
    }
};

struct Beam {
    int width; //横
    int height;  //縦
    int s_rate, c_rate;
    vector<vector<int>> relate; //それぞれの座標に対する隣接リスト
    vector<vector<int>> ntw_relate; //壁をすり抜けない隣接リスト

    set predata[be_size];
    set nextdata[be_size];



    void be_search(vector<int> board, int selectable, int n_select, int recent, int n_val, int pre_root) {    //n_selectは現在選択しているピースの座標
        int i, j, k;
        int valu;
        int c;  //交換対象の座標
        //選択回数が残っていて、現在選択しているピースが正解位置にあるなら選択操作を行う
        //一回目は選択操作を行わないようにするため、recentの初期値-1の時選択操作を行わない
        if (selectable > 0 && (recent == -1 || piece_val(n_select, board.at(n_select),0) == 0)) {
            for (i = 0; i < width * height; i++) {
                if (i != board.at(i) && i != n_select) {   //すでに揃っているピース,現在選択しているピースを選択しない
                    for (j = 0; j < 4; j++) {   //各方向の探索
                            if (relate.at(i).at(j) != -1) {
                                c = relate.at(i).at(j);
                                //盤面値の更新
                                valu = n_val - (piece_val(c, board.at(c),0) + piece_val(i, board.at(i),0)) + (piece_val(c, board.at(i),0) + piece_val(i, board.at(c),0));
                                if (selectable != 1)
                                    node.emplace(valu, make_pair((i + 1) * 1000 + j, pre_root));
                                else {
                                    //選択回数が残り1なら今回の選択で使い切っているので、
                                    swap(board.at(i), board.at(c));
                                    //交換前後での整合性をチェックする
                                    if (parity(board, height * width) == 0)
                                        node.emplace(valu, make_pair((i + 1) * 1000 + j, pre_root));
                                    swap(board.at(i), board.at(c));
                                }
                                //盤面値の更新終了
                            }
                    }
                }
            }
        }
        if (recent != -1) {
            for (k = 0; k < 4; k++) {
                if (selectable != 0) {
                    if (abs(k - recent) != 2 && relate.at(n_select).at(k) != -1) {
                        c = relate.at(n_select).at(k);
                        valu = n_val - (piece_val(c, board.at(c),0) + piece_val(n_select, board.at(n_select),0)) + (piece_val(c, board.at(n_select),0) + piece_val(n_select, board.at(c),0));
                        if (selectable != 0)
                            node.emplace(valu, make_pair(k, pre_root));
                        else {
                            swap(board.at(n_select), board.at(c));
                            if (parity(board, height * width) == 0)
                                node.emplace(valu, make_pair(k, pre_root));
                            swap(board.at(n_select), board.at(c));
                        }
                    }
                }
                else {  //選択回数が0なら、壁を越えない移動をする
                    if (abs(k - recent) != 2 && ntw_relate.at(n_select).at(k) != -1) {
                        c = ntw_relate.at(n_select).at(k);
                        valu = n_val - (piece_val(c, board.at(c),0) + piece_val(n_select, board.at(n_select),0)) + (piece_val(c, board.at(n_select),0) + piece_val(n_select, board.at(c),0));
                        if (c == board.at(c)) valu += wro;
                        if (n_select == board.at(n_select)) valu += wro;
                        if (n_select == board.at(c)) valu -= wro;
                        if (c == board.at(n_select)) valu -= wro;
                        if (selectable != 0)
                            node.emplace(valu, make_pair(k, pre_root));
                        else {
                            swap(board.at(n_select), board.at(c));
                            if (parity(board, height * width) == 0)
                                node.emplace(valu, make_pair(k, pre_root));
                            swap(board.at(n_select), board.at(c));
                        }
                    }
                }
            }
        }
    }
    void be_finished(queue<int> moved) {    //探索成功
        int i, j;
        int s_count = 0; //選択回数のカウンター
        queue<int> s_rec;    //選択座標の記録
        int count;  //交換回数のカウンター
        queue<int> c_count; //交換回数のカウントの記録
        queue<int> c_rec;  //交換方向の記録

        FILE* outfile;
        outfile = fopen("output.txt", "w");
        fprintf(outfile, "%s\n", rot);
        while (moved.empty() == false) {
            s_count++;
            s_rec.push(((moved.front() - 1000) / width) + ((moved.front() - 1000) % width) * 16);
            moved.pop();
            count = 0;
            while (moved.empty() == false) {
                if (moved.front() > 4) break;   //選択操作ならbreak
                count++;
                c_rec.push(moved.front());
                moved.pop();
            }
            c_count.push(count);
        }
        fprintf(outfile, "%d\n", s_count); //cout << s_count << endl;
        while (c_rec.empty() == false) {
            if (c_count.front() == 0) {
                s_rec.pop();
                c_count.pop();
                continue;
            }
            fprintf(outfile, "%02x\n", s_rec.front()); //printf("%02x\n", s_rec.front());
            s_rec.pop();
            fprintf(outfile, "%d\n", c_count.front()); //cout << c_count.front() << endl;
            for (i = 0; i < c_count.front(); i++) {
                switch (c_rec.front()) {
                case 0:
                    fprintf(outfile, "U"); //cout << 'U';
                    break;
                case 1:
                    fprintf(outfile, "R"); //cout << 'R';
                    break;
                case 2:
                    fprintf(outfile, "D"); //cout << 'D';
                    break;
                case 3:
                    fprintf(outfile, "L"); //cout << 'L';
                    break;
                }
                c_rec.pop();
            }
            fprintf(outfile, "\n"); //cout << endl;
            c_count.pop();
        }
        fclose(outfile);
    }
    int piece_val(int np, int cp,int mode) {  //np->現在位置cp->正解位置
        int cx = cp % width;
        int cy = cp / width;
        int nx = np % width;
        int ny = np / width;
        int val = min(abs(nx - cx), width - abs(nx - cx)) + min(abs(ny - cy), height - abs(ny - cy));
        if (mode == 1)
            return val;
        int noThroughVal = abs(nx - cx) + abs(ny - cy);
        //int poswei = abs(((int)width / 2) - cx) + (((int)height / 2) - cy);
        //if (np == cp) poswei = 0;
        return (val*manhattan) + (val * val*pow2manhattan) +(noThroughVal+ntw)+(noThroughVal * noThroughVal * pow2ntw);
    }
    int wrong_piece(vector<int> board) {
        int i,wrong=0;
        for (i = 0; i < height * width; i++) {
            if (i != board.at(i)) wrong++;
        }
        return wrong;
    }
    Beam(vector<vector<int>> rel, vector<vector<int>> ntw_rel, int w, int h, int sr, int cr) {
        relate = rel;
        ntw_relate = ntw_rel;
        width = w;
        height = h;
        s_rate = sr;
        c_rate = cr;
    }
};

void inputdata() {
    int width, height;
    int selectable;
    int s_rate, c_rate;
    int i;

    FILE* datafile;

    datafile = fopen("C:/Procon32_Simulator/puzzle_solver/puzzle_text/puzzle_solution.txt", "r");

    (void)fscanf(datafile, "%d %d", &width, &height);

    vector<int> board(width * height);

    (void)fscanf(datafile, "%d", &selectable);
    (void)fscanf(datafile, "%d %d", &s_rate, &c_rate);
    for (i = 0; i < width * height; i++) {
        (void)fscanf(datafile, "%x", &board[i]);
    }
    (void)fscanf(datafile, "%s", rot);

    fclose(datafile);


    FILE* infile;
    infile = fopen("input.txt", "w");
    fprintf(infile, "%d %d\n", width, height);
    fprintf(infile, "%d\n", selectable);
    fprintf(infile, "%d %d\n", s_rate, c_rate);
    for (i = 0; i < width * height; i++) {
        fprintf(infile, "%x ", board[i]);
    }
    fprintf(infile, "\n");
    fclose(infile);
}

void outputdata() {
    FILE* outfile;
    FILE* answerfile;
    int temp;

    outfile = fopen("output.txt", "r");
    answerfile = fopen("C:/Procon32_Simulator/puzzle_solver/puzzle_text/solution.txt", "w");

    while ((temp = fgetc(outfile)) != EOF) {
        fprintf(answerfile, "%c", temp);
    }

    fclose(outfile);
    fclose(answerfile);
}

int main() {
    inputdata();

    //clock_t start = clock();
    int i, j, k;
    int width, height;
    int selectable;
    int s_rate, c_rate; //s_rateは選択コスト,c_rateは交換コスト

    FILE* infile;
    infile = fopen("input.txt", "r"); //パスを適宜変更してください
    (void)fscanf(infile, "%d %d", &width, &height); //cin >> width >> height;
    vector<int> board(width * height);
    (void)fscanf(infile, "%d", &selectable); //cin >> selectable;
    (void)fscanf(infile, "%d %d", &s_rate, &c_rate); //cin >> s_rate >> c_rate;
    for (i = 0; i < width * height; i++) {
        (void)fscanf(infile, "%x", &board[i]); //(void)scanf("%x", &board[i]);
        board.at(i) = (int)(board.at(i) / 16) + (board.at(i) % 16) * width;
    }
    fclose(infile);

    vector<vector<int>> relate(width * height, vector<int>(4));

    //relateの書き込み
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            for (k = 0; k <= 3; k++) {
                switch (k) {
                case 0: //上方向の隣接
                    if (i != 0)
                        relate.at(i * width + j).at(0) = (i - 1) * width + j;
                    else
                        relate.at(j).at(0) = (height - 1) * width + j;
                    break;
                case 1: //右方向
                    if (j != width - 1)
                        relate.at(i * width + j).at(1) = i * width + j + 1;
                    else
                        relate.at(i * width + j).at(1) = i * width;
                    break;
                case 2: //下方向
                    if (height == 2) {   //ダブり防止
                        relate.at(i * width + j).at(2) = -1;
                        break;
                    }
                    if (i != height - 1)
                        relate.at(i * width + j).at(2) = (i + 1) * width + j;
                    else
                        relate.at(i * width + j).at(2) = j;
                    break;
                case 3: //左方向
                    if (width == 2) {
                        relate.at(i * width + j).at(3) = -1;
                        break;
                    }
                    if (j != 0)
                        relate.at(i * width + j).at(3) = i * width + j - 1;
                    else
                        relate.at(i * width + j).at(3) = (i + 1) * width - 1;
                    break;
                }
            }
        }
    }
    //relateの書き込み終了


    vector<vector<int>> ntw_relate(width * height, vector<int>(4));
    //non_relateの書き込み
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            for (k = 0; k <= 3; k++) {
                switch (k) {
                case 0: //上方向の隣接
                    if (i != 0)
                        ntw_relate.at(i * width + j).at(0) = (i - 1) * width + j;
                    else
                        ntw_relate.at(j).at(0) = (height - 1) * width + j;
                    break;
                case 1: //右方向
                    if (j != width - 1)
                        ntw_relate.at(i * width + j).at(1) = i * width + j + 1;
                    else
                        ntw_relate.at(i * width + j).at(1) = i * width;
                    break;
                case 2: //下方向
                    if (height == 2) {   //ダブり防止
                        ntw_relate.at(i * width + j).at(2) = -1;
                        break;
                    }
                    if (i != height - 1)
                        ntw_relate.at(i * width + j).at(2) = (i + 1) * width + j;
                    else
                        ntw_relate.at(i * width + j).at(2) = j;
                    break;
                case 3: //左方向
                    if (width == 2) {
                        ntw_relate.at(i * width + j).at(3) = -1;
                        break;
                    }
                    if (j != 0)
                        ntw_relate.at(i * width + j).at(3) = i * width + j - 1;
                    else
                        ntw_relate.at(i * width + j).at(3) = (i + 1) * width - 1;
                    break;
                }
            }
        }
    }
    //non_relateの書き込み終了

    Beam problem(relate, ntw_relate, width, height, s_rate, c_rate);

    int val = 0;
    for (i = 0; i < width * height; i++) {
        val += problem.piece_val(i, board.at(i),0);
    }
    val += problem.wrong_piece(board) * wro;

    int startval = 0;
    for (i = 0; i < height * width; i++)
        startval += problem.piece_val(i,board.at(i), 1);
    cout << "start=" << startval<< endl;


    for (i = 0; i < height * width; i++) {
        cout << problem.piece_val(i, board.at(i), 1) << " ";
        if ((i + width) % width == width - 1) cout << endl;
    }

    problem.predata[0].board = board;
    problem.predata[0].val = val;
    problem.predata[0].selectable = selectable;
    problem.predata[0].n_select = 0;
    problem.predata[0].recent = -1;


    problem.be_search(board, selectable, -1, -1, val, 0);



    int node_count;
    int depth = 0;
    int l;

    while (gCount == 0&&depth<500) {
        int dir = 0, sel = 0, root = 0, c = 0;
        node_count = 0;
        for (auto itr = node.begin(); itr != node.end(); ++itr) {
            root = itr->second.second;  //  遷移元の指定
            if (itr->second.first >= 1000 || depth == 0) {      //選択+交換操作
                dir = itr->second.first % 1000;
                sel = itr->second.first;


                sel -= dir;
                sel = (sel / 1000) - 1;


                c = relate.at(sel).at(dir);
                problem.nextdata[node_count].moved = problem.predata[root].moved;
                problem.nextdata[node_count].moved.push(sel + 1000);
                problem.nextdata[node_count].moved.push(dir);
                problem.nextdata[node_count].board = problem.predata[root].board;
                swap(problem.nextdata[node_count].board.at(sel), problem.nextdata[node_count].board.at(c));

                problem.nextdata[node_count].val = itr->first;
                problem.nextdata[node_count].selectable = problem.predata[root].selectable - 1;
                problem.nextdata[node_count].n_select = c;
                problem.nextdata[node_count].recent = dir;
            }
            else {                                          //交換操作
                dir = itr->second.first;
                sel = problem.predata[root].n_select;

                c = relate.at(sel).at(dir);
                problem.nextdata[node_count].moved = problem.predata[root].moved;
                problem.nextdata[node_count].moved.push(dir);
                problem.nextdata[node_count].board = problem.predata[root].board;
                swap(problem.nextdata[node_count].board.at(sel), problem.nextdata[node_count].board.at(c));

                problem.nextdata[node_count].val = itr->first;
                problem.nextdata[node_count].selectable = problem.predata[root].selectable;
                problem.nextdata[node_count].n_select = c;
                problem.nextdata[node_count].recent = dir;
            }
            node_count++;
            if (node_count == be_size) break;
        }
        depth++;
        cout << "depth=" << depth << endl;

        for (i = 0; i < node_count; i++) {
            problem.predata[i] = problem.nextdata[i];
        }

        node.clear();

        for (l = 0; l < node_count; l++) {
            if (gCount == 0 && problem.wrong_piece(problem.predata[l].board) == 0) {
                problem.be_finished(problem.predata[l].moved);
                gCount++;
                break;
            }

            problem.be_search(problem.predata[l].board, problem.predata[l].selectable, problem.predata[l].n_select, problem.predata[l].recent, problem.predata[l].val, l);
        }
    }
    outputdata();
    int result = 0;
    for (i = 0; i < height*width; i++)
        result += problem.piece_val(i,problem.nextdata->board.at(i), 1);
    cout <<"result="<< result<<endl;
    for (i = 0; i < 10000000; i++)
        cout << 'e';
    return 0;
}

int parity(vector<int> board, int size) {
    int i;
    int turning = 0;
    bool nup = true;
    for (i = 0; i < size - 1; i++) {
        if (nup) {
            if (board.at(i) > board.at(i + 1)) {
                nup = false;
                turning++;
            }
        }
        else {
            if (board.at(i) < board.at(i + 1)) {
                nup = true;
                turning++;
            }
        }
    }
    return turning % 2;
}
