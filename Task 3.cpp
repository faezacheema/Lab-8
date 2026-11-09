#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

enum Criterion { MAXIMUM = 1, MINIMUM = 2, SUM = 3 };

struct Node {
    int key;
    int count;       // number of copies of 'key'
    int height;      // height of the subtree rooted here (leaf = 1)
    Node* left, * right;
    Node(int k) : key(k), count(1), height(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
    Node* root;
    int totalElements;      // counts duplicates too
    int rotations;          // total rotations performed (for the report)

    // ---------- basic helpers ----------
    static int height(Node* n) { return n ? n->height : 0; }
    static int balanceFactor(Node* n) { return n ? height(n->left) - height(n->right) : 0; }
    static void updateHeight(Node* n) { n->height = 1 + max(height(n->left), height(n->right)); }

    // ---------- rotations ----------
    Node* rotateRight(Node* y) {          
        Node* x = y->left;               
        y->left = x->right;               
        x->right = y;                      
        updateHeight(y);                   
        updateHeight(x);
        rotations++;
        return x;
    }

    Node* rotateLeft(Node* x) {            
        Node* y = x->right;                
        x->right = y->left;                
        y->left = x;                       
        updateHeight(x);                   
        updateHeight(y);
        rotations++;
        return y;
    }

    // ---------- the "healing" step ----------
    // Called on the way back up after every insert / delete.
    Node* rebalance(Node* n) {
        updateHeight(n);
        int bf = balanceFactor(n);

        if (bf > 1) {                                         // left heavy
            if (balanceFactor(n->left) >= 0) {                // Left-Left
                cout << "   [heal] Left-Left case at " << n->key
                    << " -> right rotation\n";
                return rotateRight(n);
            }
            cout << "   [heal] Left-Right case at " << n->key       // Left-Right
                << " -> left rotation on " << n->left->key
                << ", then right rotation on " << n->key << "\n";
            n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (bf < -1) {                                        // right heavy
            if (balanceFactor(n->right) <= 0) {               // Right-Right
                cout << "   [heal] Right-Right case at " << n->key
                    << " -> left rotation\n";
                return rotateLeft(n);
            }
            cout << "   [heal] Right-Left case at " << n->key        // Right-Left
                << " -> right rotation on " << n->right->key
                << ", then left rotation on " << n->key << "\n";
            n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;                                             // already balanced
    }

    // ---------- insert / delete (recursive) ----------
    Node* insert(Node* n, int key) {
        if (!n) return new Node(key);
        if (key < n->key)      n->left = insert(n->left, key);
        else if (key > n->key) n->right = insert(n->right, key);
        else { n->count++; return n; }                        // duplicate: just count it
        return rebalance(n);
    }

    static Node* minNode(Node* n) { while (n->left) n = n->left; return n; }

    // removes ONE copy of 'key'
    Node* removeOne(Node* n, int key, bool& found) {
        if (!n) return nullptr;
        if (key < n->key)      n->left = removeOne(n->left, key, found);
        else if (key > n->key) n->right = removeOne(n->right, key, found);
        else {
            found = true;
            if (n->count > 1) { n->count--; return n; }       // other copies remain
            // count == 1: physically remove the node
            if (!n->left || !n->right) {                      // 0 or 1 child
                Node* child = n->left ? n->left : n->right;
                delete n;
                return child;                                 // (null child -> leaf removed)
            }
            Node* succ = minNode(n->right);                   // 2 children: in-order successor
            n->key = succ->key;
            n->count = succ->count;
            succ->count = 1;                                  // make sure succ node is deleted
            bool dummy;
            n->right = removeOne(n->right, succ->key, dummy);
        }
        return rebalance(n);
    }

    // ---------- traversals / printing ----------
    // Reverse in-order: right subtree on top, left on the bottom.
    void printSideways(Node* n, int depth) const {
        if (!n) return;
        printSideways(n->right, depth + 1);
        cout << string(depth * 7, ' ') << n->key;
        if (n->count > 1) cout << "x" << n->count;
        cout << " (h=" << n->height << ",bf=" << balanceFactor(n) << ")\n";
        printSideways(n->left, depth + 1);
    }

    void inorder(Node* n, vector<int>& out) const {           // ascending, duplicates expanded
        if (!n) return;
        inorder(n->left, out);
        for (int i = 0; i < n->count; i++) out.push_back(n->key);
        inorder(n->right, out);
    }

    void preorder(Node* n, vector<int>& out) const {
        if (!n) return;
        out.push_back(n->key);
        preorder(n->left, out);
        preorder(n->right, out);
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    // collect up to k elements walking from the largest key downwards
    void collectLargest(Node* n, int k, vector<int>& out) const {
        if (!n || (int)out.size() >= k) return;
        collectLargest(n->right, k, out);
        for (int i = 0; i < n->count && (int)out.size() < k; i++) out.push_back(n->key);
        collectLargest(n->left, k, out);
    }

    // collect up to k elements walking from the smallest key upwards
    void collectSmallest(Node* n, int k, vector<int>& out) const {
        if (!n || (int)out.size() >= k) return;
        collectSmallest(n->left, k, out);
        for (int i = 0; i < n->count && (int)out.size() < k; i++) out.push_back(n->key);
        collectSmallest(n->right, k, out);
    }

public:
    AVLTree() : root(nullptr), totalElements(0), rotations(0) {}
    ~AVLTree() { destroy(root); }

    int size() const { return totalElements; }
    int rotationCount() const { return rotations; }

    void print() const {
        if (!root) { cout << "   (tree is empty)\n"; return; }
        cout << "   Tree (root at far left, right subtree on top):\n";
        printSideways(root, 1);
        vector<int> pre, in;
        preorder(root, pre);
        inorder(root, in);
        cout << "   Preorder : ";  for (int x : pre) cout << x << " ";  cout << "\n";
        cout << "   Inorder  : ";  for (int x : in)  cout << x << " ";  cout << "\n";
    }

    // Insert one element, rebalance automatically, then print the tree.
    void insert(int key) {
        cout << "Insert " << key << ":\n";
        root = insert(root, key);
        totalElements++;
        print();
        cout << "\n";
    }

    void insertMany(const vector<int>& keys) {
        for (int k : keys) insert(k);
    }

    // Returns the top-k elements for a criterion (and the sum for SUM).
    vector<int> topK(int k, Criterion c, long long* sumOut = nullptr) const {
        vector<int> res;
        if (c == MINIMUM) collectSmallest(root, k, res);
        else              collectLargest(root, k, res);       // MAXIMUM and SUM
        if (sumOut) {
            long long s = 0;
            for (int x : res) s += x;
            *sumOut = s;
        }
        return res;
    }

    void queryTopK(int k, Criterion c) const {
        long long sum = 0;
        vector<int> res = topK(k, c, &sum);
        const char* name = (c == MAXIMUM) ? "MAXIMUM (k largest)"
            : (c == MINIMUM) ? "MINIMUM (k smallest)"
            : "SUM (sum of the k largest)";
        cout << "Query top-" << k << " by " << name << ": ";
        if (res.empty()) { cout << "(no elements)\n\n"; return; }
        for (int x : res) cout << x << " ";
        if (c == SUM) cout << "  => sum = " << sum;
        cout << "\n\n";
    }

    // Delete one copy of 'key' (rebalances and prints the tree afterwards).
    bool remove(int key) {
        cout << "Delete " << key << ":\n";
        bool found = false;
        root = removeOne(root, key, found);
        if (found) totalElements--;
        else cout << "   (" << key << " not found)\n";
        print();
        cout << "\n";
        return found;
    }

    // Delete the top-k elements one by one; tree is rebalanced + printed after each.
    void deleteTopK(int k, Criterion c) {
        vector<int> victims = topK(k, c);
        cout << "=== Deleting top-" << k << " elements: ";
        for (int x : victims) cout << x << " ";
        cout << "===\n";
        for (int x : victims) remove(x);
    }

    // Update = remove the old value, insert the new one (both rebalance).
    void update(int oldKey, int newKey) {
        cout << "=== Update " << oldKey << " -> " << newKey << " ===\n";
        if (remove(oldKey)) insert(newKey);
    }
};

// ---------- scripted demo used for the report ----------
void runDemo() {
    AVLTree t;
    cout << "##### 1. INSERTIONS (tree is rebalanced after every insert) #####\n\n";
    t.insertMany({ 30, 20, 10, 40, 50, 25, 35, 60, 28, 28 });

    cout << "##### 2. TOP-K QUERIES (k = 3) #####\n\n";
    t.queryTopK(3, MAXIMUM);
    t.queryTopK(3, MINIMUM);
    t.queryTopK(3, SUM);

    cout << "##### 3. UPDATE #####\n\n";
    t.update(35, 5);

    cout << "##### 4. DELETE TOP-3 (by maximum) #####\n\n";
    t.deleteTopK(3, MAXIMUM);

    cout << "##### 5. DELETE TOP-2 (by minimum) #####\n\n";
    t.deleteTopK(2, MINIMUM);

    cout << "Total rotations performed: " << t.rotationCount() << "\n";
}

// ---------- interactive menu ----------
Criterion askCriterion() {
    int c;
    cout << "Criterion  1) Maximum  2) Minimum  3) Sum : ";
    cin >> c;
    if (c < 1 || c > 3) c = 1;
    return (Criterion)c;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "demo") { runDemo(); return 0; }

    AVLTree t;
    int choice;
    do {
        cout << "\n===== AVL Top-K Menu =====\n"
            << "1. Insert multiple elements\n"
            << "2. Query top-k elements\n"
            << "3. Delete top-k elements\n"
            << "4. Update an element\n"
            << "5. Print tree\n"
            << "6. Exit\n"
            << "Choice: ";
        if (!(cin >> choice)) break;

        if (choice == 1) {
            int n;
            cout << "How many elements? ";
            cin >> n;
            cout << "Enter " << n << " elements: ";
            vector<int> v(n);
            for (int& x : v) cin >> x;
            cout << "\n";
            t.insertMany(v);
        }
        else if (choice == 2) {
            int k;  cout << "Enter k: ";  cin >> k;
            t.queryTopK(k, askCriterion());
        }
        else if (choice == 3) {
            int k;  cout << "Enter k: ";  cin >> k;
            t.deleteTopK(k, askCriterion());
        }
        else if (choice == 4) {
            int a, b;
            cout << "Old value and new value: ";
            cin >> a >> b;
            t.update(a, b);
        }
        else if (choice == 5) {
            t.print();
        }
    } while (choice != 6);

    return 0;
}
