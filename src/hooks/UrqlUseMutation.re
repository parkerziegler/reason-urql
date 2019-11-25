/**
 * The type of executeMutation – a function returned by
 * useMutation to imperatively execute the mutation.
 */
type executeMutationJs =
  option(Js.Json.t) => Js.Promise.t(UrqlClient.ClientTypes.operationResult);

[@bs.module "urql"]
external useMutationJs:
  string => (UrqlTypes.jsResponse(Js.Json.t), executeMutationJs) =
  "useMutation";

/**
 * A function for converting the response to useQuery from the JavaScript
 * representation to a typed Reason record.
 */
let urqlResponseToReason =
    (parse: Js.Json.t => 'response, result: UrqlTypes.jsResponse(Js.Json.t))
    : UrqlTypes.hookResponse('response) => {
  let data =
    result->UrqlTypes.jsDataGet->Js.Nullable.toOption->Belt.Option.map(parse);
  let error =
    result
    ->UrqlTypes.jsErrorGet
    ->Belt.Option.map(UrqlCombinedError.combinedErrorToRecord);
  let fetching = result->UrqlTypes.fetchingGet;

  let response =
    switch (fetching, data, error) {
    | (true, _, _) => UrqlTypes.Fetching
    | (false, _, Some(error)) => Error(error)
    | (false, Some(data), _) => Data(data)
    | (false, None, None) => NotFound
    };

  {fetching, data, error, response};
};

/**
 * The useMutation hook.
 *
 * Accepts the following arguments:
 *
 * request – a tuple containing the query, a parse function for decoding the
 * JSON response, and a higher-order function that calls a passed-in function
 * with the variables corresponding to the GraphQL. This should be generated by
 * graphql_ppx_re and is accessible in the MyQuery.definition variable.
 */;
let useMutation = ((parse, query, composeVariables)) => {
  composeVariables(variables => {
    let (responseJs, executeMutationJs) = useMutationJs(query);

    let response =
      React.useMemo2(
        () => responseJs |> urqlResponseToReason(parse),
        (parse, responseJs),
      );

    let executeMutation =
      React.useCallback1(
        () => executeMutationJs(Some(variables)),
        [|variables|],
      );

    (response, executeMutation);
  });
};

let useDynamicMutation = definition => {
  let (parse, query, composeVariables) = definition;
  let (responseJs, executeMutationJs) = useMutationJs(query);

  let response =
    React.useMemo1(
      () => responseJs |> urqlResponseToReason(parse),
      [|responseJs|],
    );

  let executeMutation =
    React.useMemo1(
      () => composeVariables(request => executeMutationJs(Some(request))),
      [|executeMutationJs|],
    );

  (response, executeMutation);
};
