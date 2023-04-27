#ifndef APPLICATION_EXAMPLE_0_BASE_HPP
#define APPLICATION_EXAMPLE_0_BASE_HPP

#include "application.hpp"

const string dataPath = "data/simple";

void Application::init_relations() {
    clear_relations();

    #if defined(RELATION_R1_STATIC)
        relations.push_back(std::unique_ptr<IRelation>(
            new EventDispatchableRelation<R1_entry>(
                "R1", dataPath + "/R1.tbl", '|', true,
                [](dbtoaster::data_t& data) {
                    return [&](R1_entry& t) {
                        data.on_insert_R1(t);
                    };
                }
        )));
    #elif defined(RELATION_R1_DYNAMIC) && defined(BATCH_SIZE)
        typedef const std::vector<DELTA_R1_entry>::iterator CIteratorR1;
        relations.push_back(std::unique_ptr<IRelation>(
            new BatchDispatchableRelation<DELTA_R1_entry>(
                "R1", dataPath + "/R1.tbl", '|', false,
                [](dbtoaster::data_t& data) {
                    return [&](CIteratorR1& begin, CIteratorR1& end) {
                        data.on_batch_update_R1(begin, end);
                    };
                }
        )));
    #elif defined(RELATION_R1_DYNAMIC)
        relations.push_back(std::unique_ptr<IRelation>(
            new EventDispatchableRelation<R1_entry>(
                "R1", dataPath + "/R1.tbl", '|', false,
                [](dbtoaster::data_t& data) {
                    return [&](R1_entry& t) {
                        data.on_insert_R1(t);
                    };
                }
        )));
    #endif

    #if defined(RELATION_R2_STATIC)
        relations.push_back(std::unique_ptr<IRelation>(
            new EventDispatchableRelation<R2_entry>(
                "R2", dataPath + "/R2.tbl", '|', true,
                [](dbtoaster::data_t& data) {
                    return [&](R2_entry& t) {
                        data.on_insert_R2(t);
                    };
                }
        )));
    #elif defined(RELATION_R2_DYNAMIC) && defined(BATCH_SIZE)
        typedef const std::vector<DELTA_R2_entry>::iterator CIteratorR2;
        relations.push_back(std::unique_ptr<IRelation>(
            new BatchDispatchableRelation<DELTA_R2_entry>(
                "R2", dataPath + "/R2.tbl", '|', false,
                [](dbtoaster::data_t& data) {
                    return [&](CIteratorR2& begin, CIteratorR2& end) {
                        data.on_batch_update_R2(begin, end);
                    };
                }
        )));
    #elif defined(RELATION_R2_DYNAMIC)
        relations.push_back(std::unique_ptr<IRelation>(
            new EventDispatchableRelation<R2_entry>(
                "R2", dataPath + "/R2.tbl", '|', false,
                [](dbtoaster::data_t& data) {
                    return [&](R2_entry& t) {
                        data.on_insert_R2(t);
                    };
                }
        )));
    #endif

    #if defined(RELATION_R3_STATIC)
        relations.push_back(std::unique_ptr<IRelation>(
            new EventDispatchableRelation<T_entry>(
                "R3", dataPath + "/R3.tbl", '|', true,
                [](dbtoaster::data_t& data) {
                    return [&](R3_entry& t) {
                        data.on_insert_R3(t);
                    };
                }
        )));
    #elif defined(RELATION_R3_DYNAMIC) && defined(BATCH_SIZE)
        typedef const std::vector<DELTA_R3_entry>::iterator CIteratorR3;
        relations.push_back(std::unique_ptr<IRelation>(
            new BatchDispatchableRelation<DELTA_R3_entry>(
                "R3", dataPath + "/R3.tbl", '|', false,
                [](dbtoaster::data_t& data) {
                    return [&](CIteratorR3& begin, CIteratorR3& end) {
                        data.on_batch_update_R3(begin, end);
                    };
                }
        )));
    #elif defined(RELATION_R3_DYNAMIC)
        relations.push_back(std::unique_ptr<IRelation>(
            new EventDispatchableRelation<R3_entry>(
                "R3", dataPath + "/R3.tbl", '|', false,
                [](dbtoaster::data_t& data) {
                    return [&](R3_entry& t) {
                        data.on_insert_R3(t);
                    };
                }
        )));
    #endif
}

#endif /* APPLICATION_SIMPLE_BASE_HPP */