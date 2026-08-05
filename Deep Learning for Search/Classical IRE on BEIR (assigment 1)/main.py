import math
import re

import pandas as pd
from tqdm import tqdm
from collections import Counter, defaultdict

from beir import util
from beir.datasets.data_loader import GenericDataLoader


from nltk.corpus import stopwords
from nltk.stem import PorterStemmer

STOP_WORDS = set(stopwords.words("english"))

stemmer = PorterStemmer()

# Load data:
dataset = "nfcorpus"
url = f"https://public.ukp.informatik.tu-darmstadt.de/thakur/BEIR/datasets/{dataset}.zip"

data_path = util.download_and_unzip(url, "datasets")

corpus, queries, qrels = GenericDataLoader(data_folder=data_path).load(split="test")


# Tokenization:
def tokenize_basic(text):

    text = text.lower()
    text = re.sub(r"[^a-z0-9\s]", " ", text)

    return text.split()

def tokenize(text):
    return tokenize_basic(text)

def tokenize_stopwords(text):

    tokens = tokenize_basic(text)

    return [token for token in tokens if token not in STOP_WORDS]

def tokenize_stemming(text):

    tokens = tokenize_stopwords(text)

    return [stemmer.stem(token) for token in tokens]

# Index
def build_index(corpus):

    index = defaultdict(list)

    doc_lengths = {}

    total_tokens = 0

    for doc_id, document in corpus.items():

        text = (document.get("title", "") + " " + document.get("text", ""))

        tokens = tokenize(text)

        doc_lengths[doc_id] = len(tokens)

        total_tokens += len(tokens)

        term_counts = Counter(tokens)

        for term, tf in term_counts.items():
            index[term].append((doc_id, tf))

    avgdl = total_tokens / len(corpus)

    return index, doc_lengths, avgdl


index, doc_lengths, avgdl = build_index(corpus)

N = len(corpus)

df_table = { term: len(postings) for term, postings in index.items()}

posting_lookup = {}

for term, postings in index.items():
    posting_lookup[term] = {
        doc_id: tf
        for doc_id, tf in postings
    }

# bool or
def rank_boolean_or(query, top_k=10):

    scores = defaultdict(int)

    for term in tokenize(query):

        if term not in index:
            continue

        for doc_id, _ in index[term]:
            scores[doc_id] += 1

    ranked = sorted(scores.items(), key=lambda x: x[1], reverse=True)

    return ranked[:top_k]


# TF-IDF
def tfidf_score(query, doc_id):

    score = 0.0

    for term in tokenize(query):

        if term not in index: continue

        idf = math.log(N / df_table[term])

        for posting_doc, tf in index[term]:
            if posting_doc == doc_id:
                score += tf * idf
                break
    return score


def rank_tfidf(query, top_k=10):

    candidate_docs = set()

    for term in tokenize(query):

        if term in index:

            for doc_id, _ in index[term]:
                candidate_docs.add(doc_id)

    scored = []

    for doc_id in candidate_docs:
        scored.append(
            (doc_id, tfidf_score(query, doc_id))
        )

    scored.sort(key=lambda x: x[1], reverse=True)

    return scored[:top_k]

# BM25
def bm25_score(query, doc_id, k1=1.5, b=0.75):

    score = 0.0

    doc_len = doc_lengths[doc_id]

    for term in tokenize(query):

        if term not in posting_lookup: continue

        tf = posting_lookup[term].get(doc_id, 0)

        if tf == 0: continue

        df = df_table[term]
        idf = math.log(1 + (N - df + 0.5) / (df + 0.5))
        score += (idf * tf * (k1 + 1) / (tf + k1 * (1 - b + b * doc_len / avgdl)))

    return score

def rank_bm25( query, top_k=10, k1=1.5, b=0.75):

    candidate_docs = set()

    for term in tokenize(query):

        if term in index:

            for doc_id, _ in index[term]: candidate_docs.add(doc_id)

    scored = []

    for doc_id in candidate_docs:

        scored.append((doc_id, bm25_score(query, doc_id, k1, b)))

    scored.sort(key=lambda x: x[1], reverse=True)

    return scored[:top_k]

# RRF
def rank_rrf(query, top_k=10, fusion_k=60):

    bm25_rank = rank_bm25(query, top_k=100)
    tfidf_rank = rank_tfidf(query, top_k=100)

    scores = defaultdict(float)

    for rank, (doc_id, _) in enumerate(bm25_rank, start=1):
        scores[doc_id] += (1 / (fusion_k + rank))

    for rank, (doc_id, _) in enumerate(tfidf_rank, start=1):
        scores[doc_id] += (1 / (fusion_k + rank))

    ranked = sorted(scores.items(), key=lambda x: x[1], reverse=True)

    return ranked[:top_k]

# nDCG@10
def dcg_at_k(ranked_docs, relevant_docs, k=10):
    dcg = 0.0

    for rank, doc_id in enumerate(ranked_docs[:k], start=1):

        rel = relevant_docs.get(doc_id, 0)

        gain = (2 ** rel) - 1

        discount = math.log2(rank + 1)

        dcg += gain / discount

    return dcg


def ndcg_at_k(ranked_docs, relevant_docs, k=10):

    actual = dcg_at_k(ranked_docs, relevant_docs, k)

    ideal_docs = sorted(relevant_docs.keys(), key=lambda d: relevant_docs[d], reverse=True)

    ideal = dcg_at_k(ideal_docs, relevant_docs, k)

    if ideal == 0: return 0

    return actual / ideal

# Map
def average_precision(ranked_docs, relevant_docs):

    precision_sum = 0

    found = 0

    for rank, doc_id in enumerate(ranked_docs, start=1):

        if relevant_docs.get(doc_id, 0) > 0:

            found += 1
            precision_sum += (found / rank)

    if found == 0: return 0

    return precision_sum / found


# Mrr
def reciprocal_rank(ranked_docs, relevant_docs):

    for rank, doc_id in enumerate(
        ranked_docs,
        start=1
    ):

        if relevant_docs.get(doc_id, 0) > 0: return 1 / rank

    return 0

# Evaluation
def evaluate(rank_function):

    ndcg_scores = []
    map_scores = []
    mrr_scores = []

    for qid, query_text in queries.items():

        ranking = rank_function(query_text)

        docs = [doc_id for doc_id, _ in ranking]

        rels = qrels[qid]

        ndcg_scores.append(ndcg_at_k(docs, rels,10))
        map_scores.append(average_precision(docs, rels))
        mrr_scores.append(reciprocal_rank(docs, rels))

    return {
        "nDCG@10": sum(ndcg_scores) / len(ndcg_scores),
        "MAP": sum(map_scores) / len(map_scores),
        "MRR": sum(mrr_scores) / len(mrr_scores),
        "per_query_ndcg": ndcg_scores
    }

# Run rankes
results = { "Boolean OR": evaluate(rank_boolean_or), "TF-IDF": evaluate(rank_tfidf), "BM25": evaluate(rank_bm25), "RRF": evaluate(rank_rrf)
}

k1_values = [0.5, 1.0, 1.2, 1.5, 2.0, 3.0]
b_values = [0, 0.25, 0.5, 0.75, 1.0]

sweep_results = []

for k1 in tqdm(k1_values):

    for b in b_values:

        ndcg_scores = []

        for qid, query_text in queries.items():

            ranking = rank_bm25(query_text, k1=k1, b=b)
            docs = [doc_id for doc_id, _ in ranking]

            ndcg_scores.append(ndcg_at_k(docs, qrels[qid],10))

        sweep_results.append({"k1": k1, "b": b, "nDCG@10": sum(ndcg_scores) / len(ndcg_scores)})

sweep_df = pd.DataFrame(sweep_results)

print("\nTOP 10 BM25 CONFIGURATIONS\n")

print(sweep_df.sort_values("nDCG@10", ascending=False).head(10))

best_row = sweep_df.loc[sweep_df["nDCG@10"].idxmax()]

sweep_df.to_csv("bm25_sweep.csv", index=False)

print("\nBEST CONFIGURATION\n")
print(best_row)
print("\nFINAL RESULTS\n")

for name, metrics in results.items():

    print(
        f"{name:12s}"
        f" nDCG@10={metrics['nDCG@10']:.4f}"
        f" MAP={metrics['MAP']:.4f}"
        f" MRR={metrics['MRR']:.4f}"
    )

# Tokenization study
def build_index_custom(corpus, tokenizer):

    index = defaultdict(list)

    doc_lengths = {}

    total_tokens = 0

    for doc_id, document in corpus.items():

        text = (document.get("title", "") + " " + document.get("text", ""))

        tokens = tokenizer(text)
        doc_lengths[doc_id] = len(tokens)
        total_tokens += len(tokens)
        term_counts = Counter(tokens)

        for term, tf in term_counts.items():
            index[term].append((doc_id, tf))

    avgdl = total_tokens / len(corpus)

    return index, doc_lengths, avgdl


def evaluate_tokenizer(tokenizer):

    local_index, local_doc_lengths, local_avgdl = (build_index_custom(corpus, tokenizer))
    local_df_table = {term: len(postings) for term, postings in local_index.items()}
    local_posting_lookup = {term: {doc_id: tf for doc_id, tf in postings} for term, postings in local_index.items()}

    ndcgs = []
    maps = []
    mrrs = []

    for qid, query_text in queries.items():

        query_tokens = tokenizer(query_text)

        candidate_docs = set()

        for term in query_tokens:
            if term in local_index:
                for doc_id, _ in local_index[term]:
                    candidate_docs.add(doc_id)

        scored = []

        for doc_id in candidate_docs:

            score = 0.0

            doc_len = local_doc_lengths[doc_id]

            for term in query_tokens:

                if term not in local_posting_lookup:
                    continue

                tf = local_posting_lookup[term].get(doc_id, 0)

                if tf == 0: continue

                df = local_df_table[term]

                idf = math.log(1 + (N - df + 0.5) / (df + 0.5))

                score += (idf * tf * (1.5 + 1) / (tf + 1.5 * (1 - 0.75 + 0.75 * doc_len / local_avgdl)) )

            scored.append((doc_id, score))

        scored.sort(key=lambda x: x[1], reverse=True)

        docs = [doc for doc, _ in scored[:10]]

        ndcgs.append( ndcg_at_k(docs, qrels[qid], 10 ))
        maps.append(average_precision(docs, qrels[qid]))
        mrrs.append(reciprocal_rank(docs, qrels[qid]))

    return {
        "nDCG@10": sum(ndcgs) / len(ndcgs),
        "MAP": sum(maps) / len(maps),
        "MRR": sum(mrrs) / len(mrrs),
        "per_query_ndcg": ndcgs
    }


print("\nTOKENIZATION STUDY\n")

tokenization_results = {"Basic": evaluate_tokenizer(tokenize_basic),
                        "Stopwords Removed": evaluate_tokenizer(tokenize_stopwords),
                        "Porter Stemming": evaluate_tokenizer(tokenize_stemming)}

for name, metrics in tokenization_results.items():

    print(
        f"{name:20s}"
        f" nDCG@10={metrics['nDCG@10']:.4f}"
        f" MAP={metrics['MAP']:.4f}"
        f" MRR={metrics['MRR']:.4f}"
    )

from scipy.stats import ttest_rel
from scipy.stats import wilcoxon
import numpy as np

basic_ndcg = tokenization_results["Basic"]["per_query_ndcg"]

stemming_ndcg = tokenization_results["Porter Stemming"]["per_query_ndcg"]

differences = np.array(stemming_ndcg) - np.array(basic_ndcg)
mean_difference = np.mean(differences)

t_stat, t_pvalue = ttest_rel(stemming_ndcg, basic_ndcg)
w_stat, w_pvalue = wilcoxon(stemming_ndcg, basic_ndcg)

ci_low = (mean_difference - 1.96 * np.std(differences, ddof=1) / np.sqrt(len(differences)))
ci_high = (mean_difference + 1.96 * np.std(differences, ddof=1) / np.sqrt(len(differences)))


print("\nStatistical significance\n")

print(
    f"Mean difference: "
    f"{mean_difference:.6f}"
)

print(
    f"95% CI: "
    f"[{ci_low:.6f}, "
    f"{ci_high:.6f}]"
)

print(
    f"Paired t-test p-value: "
    f"{t_pvalue:.6f}"
)

print(
    f"Wilcoxon p-value: "
    f"{w_pvalue:.6f}"
)

import matplotlib.pyplot as plt

plt.figure(figsize=(8, 5))

plt.boxplot([
    results["Boolean OR"]["per_query_ndcg"],
    results["TF-IDF"]["per_query_ndcg"],
    results["BM25"]["per_query_ndcg"],
    results["RRF"]["per_query_ndcg"]
])

plt.xticks(
    [1, 2, 3, 4],
    ["Boolean OR", "TF-IDF", "BM25", "RRF"]
)

plt.ylabel("nDCG@10")

plt.title(
    "Per-query nDCG@10 Distribution"
)

plt.show()
