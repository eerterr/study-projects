import random
import numpy as np
import time
import pyarrow.parquet as pq
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import ttest_rel
from pathlib import Path
from scipy.stats import ttest_rel
from datasets import load_dataset
from sentence_transformers import SentenceTransformer

SEED = 20260605

QUESTION_COUNT = None
SLICE_SIZE = 1000

USE_CHUNK_EXPERIMENT = True
RUN_ADVANCED_RETRIEVAL = True

def build_corpus(parquet_file, keep, source_type, slice_size):
    print("\nBuilding corpus...\n")

    gold = {}
    slice_docs = []

    for batch in parquet_file.iter_batches(batch_size=1000):
        docs = batch.to_pylist()

        for d in docs:
            if d["doc_id"] in keep:
                gold[d["doc_id"]] = d
            elif d["source_type"] == source_type and len(slice_docs) < slice_size:
                slice_docs.append(d)

        print(
            f"gold={len(gold):2d}/{len(keep)} | "
            f"slice={len(slice_docs):4d}/{slice_size}"
        )

        if len(gold) == len(keep) and len(slice_docs) >= slice_size:
            break

    corpus = list(gold.values()) + slice_docs

    print("\nCorpus ready!")
    print(f"Gold docs: {len(gold)}")
    print(f"Slice docs: {len(slice_docs)}")
    print(f"Total: {len(corpus)}")

    return corpus


random.seed(SEED)
np.random.seed(SEED)

print("Loading questions...")
questions = load_dataset("onyx-dot-app/EnterpriseRAG-Bench", "questions")["test"]

selected_questions = list(questions) if QUESTION_COUNT is None else list(questions)[:QUESTION_COUNT]

keep = set()

for q in selected_questions:
    keep.update(q["expected_doc_ids"])

print(f"Questions: {len(selected_questions)}")
print(f"Gold docs: {len(keep)}")

print("\nOpening local parquet...")
parquet = pq.ParquetFile("data/test.parquet")
print("Done!")

corpus = build_corpus(
    parquet_file=parquet,
    keep=keep,
    source_type="github",
    slice_size=SLICE_SIZE
)

print("\nPreparing texts...\n")

DEFAULT_CHUNK_SIZE = 256
DEFAULT_OVERLAP = 64

CHUNK_SIZE = DEFAULT_CHUNK_SIZE
OVERLAP = DEFAULT_OVERLAP

def chunk_text(text):
    chunks = []

    start = 0

    while start < len(text):

        end = min(start + CHUNK_SIZE, len(text))

        chunks.append(text[start:end])

        if end == len(text):
            break

        start = end - OVERLAP

    return chunks

def prepare_chunks(corpus, chunk_size, overlap):

    global CHUNK_SIZE
    global OVERLAP

    CHUNK_SIZE = chunk_size
    OVERLAP = overlap

    doc_ids = []
    parent_doc = []
    documents = []

    for doc in corpus:

        text = f"{doc['title']}\n\n{doc['content']}"

        for chunk in chunk_text(text):

            documents.append(chunk)
            doc_ids.append(doc["doc_id"])
            parent_doc.append(doc["doc_id"])

    return documents, doc_ids, parent_doc

def encode_documents(documents):

    print("\nEncoding documents...")

    embeddings = model.encode(
        documents,
        batch_size=32,
        normalize_embeddings=True,
        convert_to_numpy=True,
        show_progress_bar=True
    )

    return embeddings


def build_index(chunk_size, overlap):

    documents, doc_ids, parent_doc = prepare_chunks(
        corpus,
        chunk_size,
        overlap
    )

    embeddings = encode_documents(documents)

    return {
        "documents": documents,
        "doc_ids": doc_ids,
        "parent_doc": parent_doc,
        "embeddings": embeddings,
        "chunk_size": chunk_size,
        "overlap": overlap
    }

def run_experiment(name, index, query_vectors):

    results = retrieve(
        query_vectors,
        index
    )

    scores = evaluate(
        results,
        TOP_K
    )

    per_query_scores[name] = scores["Context Precision Per Query"]

    scores["Time"] = time.time() - start

    experiments.append({

        "Method": name,

        **scores

    })

    print(f"\n{name}")

    for metric, value in scores.items():

        if metric == "Context Precision Per Query":
            continue

        if isinstance(value, float):
            print(f"{metric:<20}: {value:.4f}")
        else:
            print(f"{metric:<20}: {value}")

    return results

start = time.time()

queries = []
for q in selected_questions:
    queries.append(q["question"])

print("\nLoading SBERT model...\n")
model = SentenceTransformer("all-MiniLM-L6-v2")

def fact_present(fact_embedding, chunk_embeddings, threshold=0.6):

    if len(chunk_embeddings) == 0:
        return False

    chunk_embeddings = np.asarray(chunk_embeddings)

    similarities = chunk_embeddings @ fact_embedding

    return similarities.max() >= threshold

index = build_index(
    DEFAULT_CHUNK_SIZE,
    DEFAULT_OVERLAP
)

documents = index["documents"]
doc_ids = index["doc_ids"]
parent_doc = index["parent_doc"]
doc_embeddings = index["embeddings"]

from collections import defaultdict

document_chunk_embeddings = defaultdict(list)

for doc_id, chunk, embedding in zip(
    doc_ids,
    documents,
    doc_embeddings
):
    document_chunk_embeddings[doc_id].append(embedding)

print("\nFirst document preview:\n")
print(documents[0][:400])

print("\nLength:", len(documents[0]))

print("\nFirst query:\n")
print(queries[0])

print()
print(selected_questions[0])

print("\nEncoding queries...")
query_embeddings = model.encode(
    queries,
    batch_size=32,
    show_progress_bar=True,
    normalize_embeddings=True,
    convert_to_numpy=True
)

print("\nEmbeddings ready!")
print("Documents shape:", doc_embeddings.shape)
print(f"Chunks: {len(documents)}")
print("Queries shape:", query_embeddings.shape)

print("\nVector norm example:")
print(np.linalg.norm(doc_embeddings[0]))

TOP_K = 10

TOP_K_VALUES = [1, 3, 5, 10, 20]

experiments = []
topk_results = []

chunk_results = []

Path("results").mkdir(exist_ok=True)
Path("figures").mkdir(exist_ok=True)

per_query_scores = {}

def retrieve_from_scores(similarities, index, top_k=TOP_K):

    results = []

    for scores in similarities:

        ranking = np.argsort(scores)[::-1]

        seen = set()
        top = []

        for idx in ranking:

            doc = index["parent_doc"][idx]

            if doc not in seen:

                seen.add(doc)
                top.append(idx)

            if len(top) == top_k:
                break

        results.append(np.array(top))

    return results


def retrieve(query_vectors, index, top_k=TOP_K):

    similarities = query_vectors @ index["embeddings"].T

    return retrieve_from_scores(
        similarities,
        index,
        top_k
    )

def build_multi_queries(queries):

    expanded = []

    for q in queries:

        expanded.append([

            q,

            f"Explain {q}",

            f"Documentation about {q}",

            f"How does {q} work?"

        ])

    return expanded

def build_hyde_queries(queries):

    hyde = []

    for q in queries:

        hyde.append(

            f"""
Question:
{q}

Hypothetical answer:

The answer to this question is that the enterprise documentation
describes the requested feature, explains how it works, provides
configuration details, API usage, requirements, limitations, and
practical examples needed to solve the user's problem.
""".strip()

        )

    return hyde

def retrieve_multi_query(query_groups, index):

    similarities = []

    for group in query_groups:

        embeddings = model.encode(

            group,

            normalize_embeddings=True,
            convert_to_numpy=True

        )

        group_scores = embeddings @ index["embeddings"].T

        similarities.append(

            group_scores.max(axis=0)

        )

    similarities = np.vstack(similarities)

    return retrieve_from_scores(

        similarities,

        index

    )

def retrieve_rag_fusion(query_groups, index, k_rrf=20):

    results = []

    for group in query_groups:

        embeddings = model.encode(
            group,
            normalize_embeddings=True,
            convert_to_numpy=True
        )

        similarities = embeddings @ index["embeddings"].T

        rrf_scores = np.zeros(len(index["documents"]))

        for scores in similarities:

            ranking = np.argsort(scores)[::-1]

            for rank, idx in enumerate(ranking):

                rrf_scores[idx] += 1.0 / (k_rrf + rank + 1)

        final_ranking = np.argsort(rrf_scores)[::-1]

        seen = set()
        top = []

        for idx in final_ranking:

            doc = index["parent_doc"][idx]

            if doc not in seen:

                seen.add(doc)
                top.append(idx)

            if len(top) == TOP_K:
                break

        results.append(np.array(top))

    return results

def evaluate(results, k=TOP_K):
    print("Calculating Recall...")
    recall = recall_at_k(
        results,
        selected_questions,
        k
    )

    print("Calculating MRR...")
    mrr_score = mrr(
        results,
        selected_questions
    )

    print("Calculating nDCG...")
    ndcg_score = ndcg(
        results,
        selected_questions,
        k
    )

    fact_embeddings = [
        model.encode(
            q["answer_facts"],
            normalize_embeddings=True,
            convert_to_numpy=True
        )
        for q in selected_questions
    ]

    print("Calculating Context Precision...")

    context_precision_values = context_precision(
        results,
        fact_embeddings,
        selected_questions,
        document_chunk_embeddings,
        doc_ids,
        k,
        return_per_query=True
    )

    context_precision_score = np.mean(context_precision_values)

    print("Calculating Context Recall...")

    print("Calculating Context Recall...")
    context_recall_score = context_recall(
        results,
        fact_embeddings,
        selected_questions,
        document_chunk_embeddings,
        doc_ids,
        k
    )

    return {

        f"Recall@{k}": recall,
        "MRR": mrr_score,
        f"nDCG@{k}": ndcg_score,
        "Context Precision": context_precision_score,
        "Context Recall": context_recall_score,
        "Context Precision Per Query": context_precision_values

    }

def evaluate_by_question_type(results):

    from collections import defaultdict

    groups = defaultdict(list)

    for q, r in zip(selected_questions, results):
        groups[q["question_type"]].append((q, r))

    print("\n" + "=" * 70)
    print("RESULTS BY QUESTION TYPE")
    print("=" * 70)

    for qtype in sorted(groups):

        qs = [x[0] for x in groups[qtype]]
        rs = [x[1] for x in groups[qtype]]

        if len(qs) == 0:
            continue

        recall = recall_at_k(rs, qs)
        mrr_score = mrr(rs, qs)
        ndcg_score = ndcg(rs, qs)

        fact_embeddings = [
            model.encode(
                q["answer_facts"],
                normalize_embeddings=True,
                convert_to_numpy=True
            )
            for q in qs
        ]

        cp = context_precision(
            rs,
            fact_embeddings,
            qs,
            document_chunk_embeddings,
            doc_ids
        )

        cr = context_recall(
            rs,
            fact_embeddings,
            qs,
            document_chunk_embeddings,
            doc_ids
        )

        print(f"\n{qtype}")
        print(f"Questions           : {len(qs)}")
        print(f"Recall@10           : {recall:.4f}")
        print(f"MRR                 : {mrr_score:.4f}")
        print(f"nDCG@10             : {ndcg_score:.4f}")
        print(f"Context Precision   : {cp:.4f}")
        print(f"Context Recall      : {cr:.4f}")

def recall_at_k(results, questions, k=10):

    recalls = []

    for q_idx, q in enumerate(questions):
        retrieved = {
            doc_ids[i]
            for i in results[q_idx][:k]
        }

        gold = set(q["expected_doc_ids"])

        if not gold:
            continue

        recalls.append(
            len(retrieved & gold) / len(gold)
        )

    return np.mean(recalls)


def mrr(results, questions):

    values = []

    for q_idx, q in enumerate(questions):

        gold = set(q["expected_doc_ids"])

        if len(gold) == 0:
            continue

        score = 0

        for rank, idx in enumerate(results[q_idx], start=1):

            if doc_ids[idx] in gold:
                score = 1 / rank
                break

        values.append(score)

    return np.mean(values)


def ndcg(results, questions, k=10):

    scores = []

    for q_idx, q in enumerate(questions):

        gold = set(q["expected_doc_ids"])

        if len(gold) == 0:
            continue

        dcg = 0

        for rank, idx in enumerate(results[q_idx][:k], start=1):

            if doc_ids[idx] in gold:
                dcg += 1 / np.log2(rank + 1)

        ideal = min(len(gold), k)

        idcg = 0

        for rank in range(1, ideal + 1):
            idcg += 1 / np.log2(rank + 1)

        if idcg == 0:
            scores.append(0)
        else:
            scores.append(dcg / idcg)

    return np.mean(scores)

def context_recall(results, fact_embeddings, questions, document_chunk_embeddings, doc_ids, k=10):

    values = []

    for q_idx, q in enumerate(questions):

        facts = q["answer_facts"]

        if len(facts) == 0:
            continue

        retrieved_docs = []

        for idx in results[q_idx][:k]:
            if doc_ids[idx] not in retrieved_docs:
                retrieved_docs.append(doc_ids[idx])

        question_fact_embeddings = fact_embeddings[q_idx]

        found = 0

        for fact_embedding in question_fact_embeddings:

            present = False

            for doc_id in retrieved_docs:

                if fact_present(
                    fact_embedding,
                    document_chunk_embeddings[doc_id]
                ):
                    present = True
                    break

            if present:
                found += 1

        values.append(found / len(facts))

    return np.mean(values)

def context_precision(
            results,
            fact_embeddings,
            questions,
            document_chunk_embeddings,
            doc_ids,
            k=10,
            return_per_query=False
    ):
    values = []

    for q_idx, q in enumerate(questions):

        facts = q["answer_facts"]

        if len(facts) == 0:
            continue

        relevant = []

        for idx in results[q_idx][:k]:
            if doc_ids[idx] not in relevant:
                relevant.append(doc_ids[idx])

        hits = 0
        precision_sum = 0.0

        question_fact_embeddings = fact_embeddings[q_idx]

        for rank, doc_id in enumerate(relevant, start=1):

            useful = False

            for fact_embedding in question_fact_embeddings:

                if fact_present(
                    fact_embedding,
                    document_chunk_embeddings[doc_id]
                ):
                    useful = True
                    break

            if useful:
                hits += 1
                precision_sum += hits / rank

        if hits == 0:
            values.append(0.0)
        else:
            values.append(precision_sum / hits)

    if return_per_query:
        return values

    return np.mean(values)

exact_top10 = retrieve(
    query_embeddings,
    index
)

print("\nFirst query top-10:\n")

for rank, idx in enumerate(exact_top10[0], start=1):
    print(f"{rank:2d}. doc={doc_ids[idx]}")

print("\nChecking retrieval against gold docs...\n")
hits = 0
for q_idx, q in enumerate(selected_questions):
    retrieved = {doc_ids[i] for i in exact_top10[q_idx]}
    gold = set(q["expected_doc_ids"])
    if len(gold) == 0:
        continue
    if retrieved & gold:
        hits += 1

print(f"Queries retrieving at least one gold document: {hits}/{len(selected_questions)}")

print("\nBASELINE\n")

exact_top10 = run_experiment(
    "Exact Retrieval",
    index,
    query_embeddings
)

print("\nMULTI QUERY\n")

multi_queries = build_multi_queries(
    queries
)

multi_results = retrieve_multi_query(
    multi_queries,
    index
)

scores = evaluate(
    multi_results,
    TOP_K
)

per_query_scores["MultiQuery"] = scores["Context Precision Per Query"]
experiments.append({

    "Method": "MultiQuery",

    **scores

})

print("\nMultiQuery")

for metric, value in scores.items():

    if metric == "Context Precision Per Query":
        continue

    print(f"{metric:<10}: {value:.4f}")

def run_topk_experiment():

    print("\n" + "=" * 70)
    print("TOP-K EXPERIMENT")
    print("=" * 70)

    for k in TOP_K_VALUES:

        print(f"\nTop-{k}")

        results = retrieve(
            query_embeddings,
            index,
            top_k=k
        )

        scores = evaluate(
            results,
            k
        )

        topk_results.append({

            "Top-K": k,

            **scores

        })

        for metric, value in scores.items():

            if metric == "Context Precision Per Query":
                continue

            print(f"{metric:<20}: {value:.4f}")

def run_chunk_experiment(chunk_size, overlap):

    print("=" * 70)
    print(f"Chunk={chunk_size}  Overlap={overlap}")

    documents, doc_ids, parent_doc = prepare_chunks(
        corpus,
        chunk_size,
        overlap
    )

    embeddings = model.encode(
        documents,
        batch_size=32,
        normalize_embeddings=True,
        convert_to_numpy=True,
        show_progress_bar=True
    )

    document_chunk_embeddings = defaultdict(list)

    for doc_id, embedding in zip(doc_ids, embeddings):
        document_chunk_embeddings[doc_id].append(embedding)

    total_facts = 0
    reachable_facts = 0

    for q in selected_questions:

        fact_embeddings = model.encode(
            q["answer_facts"],
            normalize_embeddings=True,
            convert_to_numpy=True
        )

        for fact_embedding in fact_embeddings:

            total_facts += 1

            present = False

            for gold_doc in q["expected_doc_ids"]:

                if gold_doc not in document_chunk_embeddings:
                    continue

                if fact_present(
                    fact_embedding,
                    document_chunk_embeddings[gold_doc]
                ):
                    present = True
                    break

            if present:
                reachable_facts += 1

    recall_floor = reachable_facts / total_facts

    print(f"Chunks       : {len(documents)}")
    print(f"Recall floor : {recall_floor:.4f}")

    return {
        "chunk": chunk_size,
        "overlap": overlap,
        "chunks": len(documents),
        "recall_floor": recall_floor
    }

if RUN_ADVANCED_RETRIEVAL:
    print("\nHyDE")

    hyde_queries = build_hyde_queries(
        queries
    )

    hyde_embeddings = model.encode(
        hyde_queries,
        normalize_embeddings=True,
        convert_to_numpy=True
    )

    hyde_results = retrieve(
        hyde_embeddings,
        index
    )

    scores = evaluate(
        hyde_results,
        TOP_K
    )

    per_query_scores["HyDE"] = scores["Context Precision Per Query"]

    experiments.append({
        "Method": "HyDE",
        **scores

    })

    for metric, value in scores.items():

        if metric == "Context Precision Per Query":
            continue

        print(f"{metric:<10}: {value:.4f}")

    print("\nRAG Fusion")

    fusion_results = retrieve_rag_fusion(
        multi_queries,
        index
    )

    scores = evaluate(
        fusion_results,
        TOP_K
    )

    per_query_scores["RAG-Fusion"] = scores["Context Precision Per Query"]

    experiments.append({
        "Method": "RAG-Fusion",
        **scores
    })

    for metric, value in scores.items():

        if metric == "Context Precision Per Query":
            continue

        print(f"{metric:<10}: {value:.4f}")

run_topk_experiment()

print("\nEXPERIMENT SUMMARY\n")

for row in experiments:
    print(
        f"{row['Method']:15}"
        f"Recall={row[f'Recall@{TOP_K}']:.4f}   "
        f"MRR={row['MRR']:.4f}   "
        f"nDCG={row[f'nDCG@{TOP_K}']:.4f}   "
        f"CtxPrec={row['Context Precision']:.4f}   "
        f"CtxRec={row['Context Recall']:.4f}"
    )

print("\n")
print("=" * 70)
print("CHUNK SIZE EXPERIMENT")
print("=" * 70)

chunk_results = []

if USE_CHUNK_EXPERIMENT:

    for chunk_size, overlap in [
        (128, 32),
        (256, 64),
        (384, 64),
        (512, 128),

    ]:
        result = run_chunk_experiment(
            chunk_size,
            overlap
        )

        chunk_results.append(result)

print("\nSUMMARY\n")

for r in chunk_results:

    print(
        f"chunk={r['chunk']:>3}   "
        f"overlap={r['overlap']:>3}   "
        f"chunks={r['chunks']:>6}   "
        f"recall_floor={r['recall_floor']:.4f}"
    )

print("\n")
print("=" * 70)
print("TOP-K SUMMARY")
print("=" * 70)

for r in topk_results:

    k = r["Top-K"]

    print(
        f"Top-{k:>2}   "
        f"Recall={r[f'Recall@{k}']:.4f}   "
        f"MRR={r['MRR']:.4f}   "
        f"nDCG={r[f'nDCG@{k}']:.4f}   "
        f"CtxPrec={r['Context Precision']:.4f}   "
        f"CtxRec={r['Context Recall']:.4f}"
    )

evaluate_by_question_type(exact_top10)

print("\nSaving CSV files...")

retrieval_df = pd.DataFrame(experiments)
retrieval_df = retrieval_df.drop(columns=["Context Precision Per Query"], errors="ignore")
retrieval_df.to_csv("results/retrieval_results.csv", index=False)

topk_df = pd.DataFrame(topk_results)
topk_df = topk_df.drop(columns=["Context Precision Per Query"], errors="ignore")
topk_df.to_csv("results/topk_results.csv", index=False)

if len(chunk_results):
    chunk_df = pd.DataFrame(chunk_results)
    chunk_df.to_csv("results/chunk_results.csv", index=False)

print("CSV saved.")

print("\nRunning statistical significance test...")

baseline = np.array(per_query_scores["Exact Retrieval"])

best_method = None
best_score = -1

for method, values in per_query_scores.items():

    score = np.mean(values)

    if method != "Exact Retrieval" and score > best_score:
        best_score = score
        best_method = method

best_values = np.array(per_query_scores[best_method])

t_stat, p_value = ttest_rel(baseline, best_values)

print(f"Best retrieval method : {best_method}")
print(f"T statistic : {t_stat:.4f}")
print(f"P-value : {p_value:.6f}")

print("\nGenerating plots...")

retrieval_df.plot(
    x="Method",
    y=[f"Recall@{TOP_K}", "MRR", f"nDCG@{TOP_K}"],
    kind="bar",
    rot=0
)

plt.tight_layout()
plt.savefig("figures/retrieval_methods.png")
plt.close()

topk_df.plot(
    x="Top-K",
    y=["Context Precision", "Context Recall"],
    marker="o"
)

plt.tight_layout()
plt.savefig("figures/topk_metrics.png")
plt.close()

if len(chunk_results):

    chunk_df.plot(
        x="chunk",
        y="recall_floor",
        marker="o"
    )

    plt.tight_layout()
    plt.savefig("figures/chunk_recall.png")
    plt.close()

print("\nDone.")
