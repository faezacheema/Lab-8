#include <iostream>
#include <vector>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int v) : val(v), next(nullptr) {}
};

// Binary min-heap of ListNode pointers 
class MinHeap {
    vector<ListNode*> a;

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (a[i]->val < a[p]->val) { swap(a[i], a[p]); i = p; }
            else break;
        }
    }

    void siftDown(int i) {
        int n = (int)a.size();
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, m = i;
            if (l < n && a[l]->val < a[m]->val) m = l;
            if (r < n && a[r]->val < a[m]->val) m = r;
            if (m == i) break;
            swap(a[i], a[m]);
            i = m;
        }
    }

public:
    bool empty() const { return a.empty(); }

    void push(ListNode* x) { a.push_back(x); siftUp((int)a.size() - 1); }

    ListNode* pop() {
        ListNode* top = a[0];
        a[0] = a.back();
        a.pop_back();
        if (!a.empty()) siftDown(0);
        return top;
    }
};

// Merge k sorted lists 
ListNode* mergeKLists(vector<ListNode*>& lists) {
    MinHeap heap;
    for (ListNode* head : lists)             // push the first node of every list
        if (head) heap.push(head);

    ListNode dummy(0);                       // dummy head simplifies appending
    ListNode* tail = &dummy;

    while (!heap.empty()) {
        ListNode* smallest = heap.pop();     // global minimum among the fronts
        tail->next = smallest;               // re-use the existing node (no copy)
        tail = smallest;
        if (smallest->next)                  // bring in its successor
            heap.push(smallest->next);
    }
    return dummy.next;
}

// Helpers 
ListNode* buildList(const vector<int>& v) {
    ListNode* head = nullptr, * tail = nullptr;
    for (int x : v) {
        ListNode* n = new ListNode(x);
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
    }
    return head;
}

void printList(ListNode* head) {
    for (ListNode* p = head; p; p = p->next) {
        cout << p->val;
        if (p->next) cout << " -> ";
    }
    cout << "\n";
}

int main() {
    int k;
    cout << "Enter number of linked lists k: ";
    cin >> k;

    vector<ListNode*> lists;
    for (int i = 0; i < k; i++) {
        int len;
        cout << "List " << i + 1 << " - enter its length, then its sorted elements: ";
        cin >> len;
        vector<int> v(len);
        for (int j = 0; j < len; j++) cin >> v[j];
        lists.push_back(buildList(v));
    }

    cout << "\nInput lists:\n";
    for (int i = 0; i < k; i++) {
        cout << "  List " << i + 1 << ": ";
        if (lists[i]) printList(lists[i]); else cout << "(empty)\n";
    }

    ListNode* merged = mergeKLists(lists);

    cout << "\nMerged sorted list:\n  ";
    if (merged) printList(merged); else cout << "(empty)\n";

    while (merged) {                         // free memory
        ListNode* t = merged;
        merged = merged->next;
        delete t;
    }
    return 0;
}
