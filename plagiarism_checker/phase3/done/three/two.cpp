class Solution {
public:
    vector<vector<string>> solveNQueens(int n) {
        vector<vector<string>> results; // To store all valid solutions
        vector<string> board(n, string(n, '.')); // Board initialized with '.'
        vector<bool> columns(n, false); // Tracks columns under attack
        vector<bool> diag1(2 * n - 1, false); // Tracks \ diagonals under attack
        vector<bool> diag2(2 * n - 1, false); // Tracks / diagonals under attack
        backtrack(0, n, board, columns, diag1, diag2, results);
        return results;
    }

private:
    void backtrack(int row, int n, vector<string>& board, vector<bool>& columns, 
                   vector<bool>& diag1, vector<bool>& diag2, vector<vector<string>>& results) {
        if (row == n) { // Base case: all queens are placed
            results.push_back(board);
            return;
        }
        for (int col = 0; col < n; ++col) {
            if (columns[col] || diag1[row - col + n - 1] || diag2[row + col]) {
                continue; // Skip invalid positions
            }
            // Place the queen
            board[row][col] = 'Q';
            columns[col] = true;
            diag1[row - col + n - 1] = true;
            diag2[row + col] = true;
            
            // Recurse to the next row
            backtrack(row + 1, n, board, columns, diag1, diag2, results);
            
            // Backtrack: remove the queen
            board[row][col] = '.';
            columns[col] = false;
            diag1[row - col + n - 1] = false;
            diag2[row + col] = false;
        }
    }
};
