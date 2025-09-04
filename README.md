## Key Value Store with Transactions


Implements an in-memory key-value store with the following operations:
- **`put(key, value)`**
- **`get(key)`** → value
- **`delete(key)`**

adds transaction capabilities:
- **`begin()`**: Start new transaction
- **`commit()`**: Apply all changes
- **`rollback()`**: Discard recent transaction

### Next:
- Test cases, Unit tests, TDD
- Should be Thread Safe
- Persistence 
- Add Cpp Implementation