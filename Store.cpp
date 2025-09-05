#include<iostream>
#include<unordered_map>
#include<set>
#include<vector>
#include<string>

template<typename KeyT, typename ValueT>
class Store {
    public:
        virtual const ValueT * const get(const KeyT & key) const;

        virtual void put(KeyT key, ValueT value);

        virtual bool remove(const KeyT & key);
};

template<typename KeyT, typename ValueT>
class TransactionStore : public Store<KeyT, ValueT>
{
    public:
        virtual void begin();

        virtual void commit();

        virtual void rollback();
};



template<typename KeyT, typename ValueT>
class KeyValueStore : public Store<KeyT, ValueT> {
    private:
        std::unordered_map<KeyT, ValueT> data;
        std::set<KeyT> deletions;
        Store<KeyT, ValueT> * baseStore = nullptr;
    public:

        KeyValueStore(Store<KeyT, ValueT> * baseStore = nullptr) {
            this->baseStore = baseStore;
        }

        void saveToBaseStore() {
            if(this->baseStore == nullptr) {
                return;
            }

            for(auto it = this->deletions.begin(); it != this->deletions.end(); ++it) {
                this->baseStore->remove(*it);
            }

            for(auto it = this->data.begin(); it != this->data.end(); ++it) {
                this->baseStore->put(it->first, it->second);
            }

            this->data.clear();
            this->deletions.clear();
        }

        const ValueT * const get(const KeyT & key) const {
            if( this->deletions.find(key) != this->deletions.end() ) {
                return nullptr;
            }
            auto it = this->data.find(key);
            if(  it != this->data.end() ) {
                return & it->second;
            }
            if(this->baseStore != nullptr) {
                return this->baseStore->get(key);
            }
            return nullptr;
        }

        void put(KeyT key, ValueT value) {
            if(this->baseStore != nullptr) {
                this->deletions.erase(key);
            }
            this->data.insert_or_assign(key, value);
        }

        bool remove(KeyT key) {
            if(this->baseStore != nullptr) {
                this->deletions.insert(key);
            }
            return this->data.erase(key) == 1;
        }
};


template<typename KeyT, typename ValueT>
class MemoryStore : public TransactionStore<KeyT, ValueT> {
    private:
        std::vector<KeyValueStore<KeyT, ValueT>*> dataLayers;
        int currIndex = 0;
    public:
        MemoryStore() {
            KeyValueStore<KeyT, ValueT> * layer = new KeyValueStore<KeyT, ValueT>();
            dataLayers.push_back(layer);
        }

        void begin() {
            KeyValueStore<KeyT, ValueT> * baseLayer = this->dataLayers[this->currIndex];
            KeyValueStore<KeyT, ValueT> * layer = new KeyValueStore<KeyT, ValueT>(baseLayer);
            this->currIndex += 1;
        }

        void commit() {
            if(this->currIndex == 0) {
                throw std::runtime_error("Cannot commit: no active transaction");
            }
            KeyValueStore<KeyT, ValueT> * transactionLayer = this->dataLayers[this->currIndex];
            transactionLayer->saveToBaseStore();
            this->dataLayers.pop_back();
            delete transactionLayer;
            this->currIndex -= 1;
        }

        void rollback() {
            if(this->currIndex == 0) {
                throw std::runtime_error("Cannot rollback: no active transaction");
            }
            KeyValueStore<KeyT, ValueT> * transactionLayer = this->dataLayers[this->currIndex];
            this->dataLayers.pop_back();
            delete transactionLayer;
            this->currIndex -= 1;
        }

        const ValueT * const get(const KeyT & key) const {
            return this->dataLayers[this->currIndex]->get(key);
        }

        void put(KeyT key, ValueT value) {
            this->dataLayers[this->currIndex]->put(key, value);
        }

        bool remove(const KeyT & key) {
            return this->dataLayers[this->currIndex]->remove(key);
        }

    
};

int main() {
    MemoryStore<std::string, int> store;

    return 0;
}