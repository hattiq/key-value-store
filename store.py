from typing import Optional, List

class Store:

    def get(self, key):
        pass

    def put(self, key, value):
        pass

    def delete(self, key):
        pass
    
class KeyValueStore(Store):

    def __init__(self, base_store: Optional[Store]):
        self.base_store = base_store
        self.deletions = set()
        self.data = {}

    def contains(self, key):
        if key not in self.data:
            if self.base_store and self.base_store.contains(key):
                return key not in self.deletions
        return key in self.data

    def reset(self):
        self.data = {}
        self.deletions = {}

    def get(self, key):
        if key not in self.data:
            if self.base_store and key not in self.deletions:
                return self.base_store.get(key)
        return self.data.get(key, None)

    def put(self, key, value):
        if self.base_store:
            self.deletions.discard(key)
        self.data[key] = value

    def delete(self, key):
        if self.base_store:
            self.deletions.add(key)
        self.data.pop(key, None)


class MemoryStore:

    def __init__(self):
        self.layers : List[KeyValueStore] = []
        self._curr_index = 0
        self.layers.append(KeyValueStore(None))

    def begin(self):
        self._curr_index += 1
        layer = KeyValueStore(self.layers[self._curr_index - 1])
        self.layers.append(layer)

    def _merge_layer(self, base_layer, transaction_layer):
        for key in transaction_layer.deletions:
            base_layer.delete(key)
        for key, value in transaction_layer.data.items():
            base_layer.put(key, value)
        transaction_layer.reset()

    def commit(self):
        if self._curr_index == 0:
            raise Exception("No active Transaction")
        transaction_layer = self.layers.pop()
        self._curr_index -= 1
        
        self._merge_layer(self.layers[self._curr_index], transaction_layer)


    def rollback(self):
        if self._curr_index == 0:
            raise Exception("No active Transaction")
        _ = self.layers.pop()
        self._curr_index -= 1

    def get(self, key):
        return self.layers[self._curr_index].get(key)

    def put(self, key, value):
        self.layers[self._curr_index].put(key, value)

    def delete(self, key):
        self.layers[self._curr_index].delete(key)


if __name__ == "__main__":

    # Basic Test
    store = MemoryStore()
    store.put("A", 1)
    assert store.get("A") == 1
    assert store.get("B") == None
    store.delete("A")
    assert store.get("A") == None

    # Transaction tests
    store = MemoryStore()
    store.put("A", 1)
    store.begin()
    store.delete("A")

    store.begin()
    store.put("B", 2)
    store.commit()
    store.put("A", 3)

    assert store.get("A") == 3

    store.delete("A")
    store.commit()
    assert store.get("A") == None
    assert store.get("B") == 2
    